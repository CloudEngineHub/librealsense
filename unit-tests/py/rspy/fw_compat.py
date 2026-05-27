# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2026 RealSense, Inc. All Rights Reserved.

"""
FW-update pre-flash compatibility gate.

For a given device, picks a candidate FW version (a user-supplied custom-fw image
or the device's RECOMMENDED_FIRMWARE_VERSION) and compares it against the device's
minimum supported FW (rs2::device::get_firmware_min_version). When the candidate is
below the min, a per-device fallback FW image can be substituted instead of skipping
or failing the test.

The fallback mapping lives in fw_fallback.json next to this file so it can be
maintained without touching code. Both run-unit-tests.py and pytest-based test
runners should import these helpers rather than re-implementing the logic.
"""

import json
import os
import re

from rspy import log


_FALLBACK_JSON = os.path.join( os.path.dirname( __file__ ), 'fw_fallback.json' )


def _load_fallback_map():
    """Read fw_fallback.json -> dict[device_name -> relpath]. Returns {} on any error."""
    try:
        with open( _FALLBACK_JSON, 'r' ) as f:
            data = json.load( f )
    except (FileNotFoundError, ValueError, OSError) as e:
        log.w( f'[fw-gate] could not load {_FALLBACK_JSON}: {e}' )
        return {}
    return data.get( 'fallbacks', {} )


def fw_fallback_image_for( rspy_device, libci_home ):
    """
    Return an absolute path to the fallback FW image for `rspy_device`, or None
    if there's no mapping or the file is missing on disk. `libci_home` is the
    root under which the relative paths in fw_fallback.json resolve.
    """
    fallbacks = _load_fallback_map()
    relpath = fallbacks.get( rspy_device.name )
    if not relpath:
        return None
    path = os.path.join( libci_home, relpath )
    if not os.path.isfile( path ):
        log.w( f'[fw-gate] fallback FW for {rspy_device.name} not found on disk: {path}' )
        return None
    return path


def version_to_tuple( s ):
    parts = re.findall( r'\d+', s or '' )
    return tuple( int( p ) for p in parts[:4] ) + (0,) * max( 0, 4 - len( parts ) )


def version_from_fw_filename( path ):
    """Mirror of test-fw-update.extract_version_from_filename; returns 'a.b.c.d' or None."""
    name = os.path.basename( path or '' )
    m = re.search( r'(\d+)_(\d+)_(\d+)_(\d+)\.(?:bin|img)$', name, re.IGNORECASE )
    if not m:
        m = re.search( r'-(\d+)\.(\d+)\.(\d+)\.(\d+)\.(?:bin|img)$', name, re.IGNORECASE )
    if not m:
        return None
    return '.'.join( m.group( i ) for i in range( 1, 5 ) )


def resolve_fw_gate( rspy_device, libci_home, test_name, sn=None,
                     custom_fw_d400_path=None, custom_fw_d555_path=None ):
    """
    Combined fw-gate check and fallback resolution for test-fw-update.

    Returns (skip: bool, fw_override: str|None):
      - (False, None)   -- compatible or no fallback found; run the test normally
      - (False, <path>) -- below min FW but fallback available; use this image instead
    """
    label = f'{rspy_device.name}_{sn}' if sn else rspy_device.name
    ok, reason = is_fw_update_compatible( rspy_device,
                                          custom_fw_d400_path=custom_fw_d400_path,
                                          custom_fw_d555_path=custom_fw_d555_path )
    log.d( f'[fw-gate] {label}: ok={ok} -- {reason}' )
    if ok:
        return False, None
    fallback = fw_fallback_image_for( rspy_device, libci_home )
    if fallback:
        if custom_fw_d400_path:
            log.i( f'{test_name}: {label} below min FW; --custom-fw-d400 is below min too, overriding with fallback {fallback}' )
        else:
            log.i( f'{test_name}: {label} below min FW; using fallback {fallback}' )
        return False, fallback
    log.w( f'{test_name}: {label} below min FW with no fallback; test will fail' )
    return False, None


def is_fw_update_compatible( rspy_device, custom_fw_d400_path=None, custom_fw_d555_path=None ):
    """
    Decide whether test-fw-update can run against `rspy_device` with the FW image
    that would be flashed. Returns (compatible: bool, reason: str). On any
    can't-decide path returns True so we err on the side of letting the test run.

    `rspy_device` is the rspy.devices.Device wrapper; the underlying pyrealsense2
    device is available at `.handle`.
    """
    try:
        import pyrealsense2 as rs
    except ImportError as e:
        return True, f'pyrealsense2 unavailable ({e})'

    handle = rspy_device.handle
    product_line = rspy_device.product_line
    product_name = rspy_device.name  # wrapper strips "Intel RealSense " prefix

    candidate = None
    source = ''
    if product_line == 'D400' and custom_fw_d400_path:
        candidate = version_from_fw_filename( custom_fw_d400_path )
        source = f'--custom-fw-d400 {os.path.basename( custom_fw_d400_path )}'
    elif 'D555' in (product_name or '') and custom_fw_d555_path:
        candidate = version_from_fw_filename( custom_fw_d555_path )
        source = f'--custom-fw-d555 {os.path.basename( custom_fw_d555_path )}'
    elif handle.supports( rs.camera_info.recommended_firmware_version ):
        candidate = handle.get_info( rs.camera_info.recommended_firmware_version )
        source = 'RECOMMENDED_FIRMWARE_VERSION'
    if not candidate:
        return True, 'no candidate FW version available; deferring to test'

    try:
        min_fw = handle.get_firmware_min_version()
    except Exception as e:
        return True, f'get_firmware_min_version raised ({e}); deferring to test'

    if not min_fw:
        return True, f'device reports no minimum FW; deferring (candidate {candidate})'

    if version_to_tuple( candidate ) >= version_to_tuple( min_fw ):
        return True, f'compatible -- candidate {candidate} >= min {min_fw} (candidate from {source})'
    return False, f'below device min FW -- candidate {candidate} < min {min_fw} (candidate from {source})'
