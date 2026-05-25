# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2026 RealSense, Inc. All Rights Reserved.

import pytest
import pyrealsense2 as rs
import numpy as np
import cv2
import logging

import sw_device as sw

log = logging.getLogger(__name__)
pytestmark = [pytest.mark.timeout(30)]

W, H = 640, 480
# average per-channel pixel diff between SDK and cv2 outputs on the same input.
# tested 0.77, choosing a low tolerance intentionally to note when a change happens
TOL_MEAN = 1

_FOUR_TWO_TWO = {rs.format.yuyv, rs.format.uyvy}


def fmt_layout(fmt):
    """Return (total_bytes, bpp) for one frame in `fmt` at W x H."""
    if fmt in _FOUR_TWO_TWO:
        return W * H * 2, 2
    return W * H * 3 // 2, 1


def cv2_decode_yuyv(raw):
    return cv2.cvtColor(raw.reshape(H, W, 2), cv2.COLOR_YUV2RGB_YUYV)


def cv2_decode_uyvy(raw):
    return cv2.cvtColor(raw.reshape(H, W, 2), cv2.COLOR_YUV2RGB_UYVY)


def cv2_decode_nv12(raw):
    return cv2.cvtColor(raw.reshape(H * 3 // 2, W), cv2.COLOR_YUV2RGB_NV12)


def cv2_decode_m420(raw):
    """cv2 has no native M420 decoder. Rearrange M420 (2 Y rows + 1 UV row, repeating)
    into NV12 byte order (Y plane then UV rows), then use cv2's NV12 decoder.
    SDK's m420_parse_one_line in src/proc/color-formats-converter.cpp interleaves UV
    as `u0 v0 u2 v2 ...` — same order NV12 expects, so reshape alone is sufficient."""
    blocks = raw.reshape(H // 2, 3, W)
    y_plane = blocks[:, :2, :].reshape(H, W)
    uv_rows = blocks[:, 2, :]
    return cv2.cvtColor(np.vstack([y_plane, uv_rows]), cv2.COLOR_YUV2RGB_NV12)


def sdk_decode(raw, fmt, decoder_cls):
    """Inject `raw` through sw_device, run SDK decoder, return RGB."""
    _, bpp = fmt_layout(fmt)
    with sw.sensor("test") as s:
        stream = s.video_stream("Color", rs.stream.color, fmt, bpp)
        s.start(stream)
        f = stream.frame()
        # stream.frame() default-fills f.pixels with a dummy buffer sized for the
        # stream's declared bpp; we replace it with our own raw bytes before publish.
        f.pixels = raw
        received = s.publish(f)
        return np.asanyarray(decoder_cls().process(received).get_data()).reshape(H, W, 3).copy()


@pytest.mark.parametrize("fmt,decoder_cls,cv2_decode", [
    (rs.format.yuyv, rs.yuy_decoder,  cv2_decode_yuyv),
    (rs.format.uyvy, rs.uyvy_decoder, cv2_decode_uyvy),
    (rs.format.nv12, rs.nv12_decoder, cv2_decode_nv12),
    (rs.format.m420, rs.m420_decoder, cv2_decode_m420),
], ids=["yuyv", "uyvy", "nv12", "m420"])
def test_sdk_vs_cv2_decoder(fmt, decoder_cls, cv2_decode):
    """Same raw bytes -> SDK decoder vs cv2 decoder. Mean diff <= TOL_MEAN.

    cv2 and the SDK decode the same bytes; big pixel diff means SDK bug.
    """
    raw = np.random.default_rng(0).integers(0, 256, size=fmt_layout(fmt)[0], dtype=np.uint8)
    rgb_sdk = sdk_decode(raw, fmt, decoder_cls)
    rgb_cv2 = cv2_decode(raw)
    mean = float(np.abs(rgb_sdk.astype(int) - rgb_cv2.astype(int)).mean())
    log.info(f"{fmt}: SDK vs cv2  mean |d|={mean:.2f}")
    assert mean <= TOL_MEAN, f"{fmt}: SDK decoder diverges from cv2 reference: mean |d|={mean:.2f} > {TOL_MEAN}"
