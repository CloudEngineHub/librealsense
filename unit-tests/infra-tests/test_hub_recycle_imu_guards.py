# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2026 RealSense, Inc. All Rights Reserved.

"""
Tests for the pre-iteration guards in unit-tests/live/hw-reset/pytest-hub-recycle-imu.py.

The live test refuses to run unless it can do a real port power-cycle through a hub.
Three branches need coverage:
- no hub at all (brainstem missing / no Acroname / no UniFi)  -> pytest.skip
- hub present, port unresolved (device not downstream of hub) -> pytest.fail
- no Motion Module on the device                              -> pytest.skip

The test file uses kebab-case, so we load it via importlib and invoke the function
directly with mocked devices/context.
"""

import importlib.util
import types
from pathlib import Path
from unittest.mock import MagicMock

import pytest

try:
    import pyrealsense2 as rs
except ImportError:
    pytestmark = pytest.mark.skip(reason="pyrealsense2 not available")
    rs = None


def _load_test_module():
    path = Path(__file__).resolve().parent.parent / "live" / "hw-reset" / "pytest-hub-recycle-imu.py"
    spec = importlib.util.spec_from_file_location("pytest_hub_recycle_imu", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _fake_dev_with_motion(sn='111'):
    """Stand-in pyrealsense2.device whose sensor list contains a Motion Module."""
    sensor = MagicMock()
    sensor.get_info.return_value = "Motion Module"
    dev = MagicMock()
    dev.query_sensors.return_value = [sensor]
    dev.get_info.return_value = sn
    return dev


class TestHubRecycleImuGuards:

    @pytest.fixture
    def mod(self):
        return _load_test_module()

    @pytest.fixture
    def fake_devices_module(self, monkeypatch, mod):
        """Mock the rspy.devices module that the test imports."""
        fake_devices = types.SimpleNamespace(
            hub=None,
            get=lambda sn: types.SimpleNamespace(port=None),
            MAX_ENUMERATION_TIME=10,
        )
        monkeypatch.setattr(mod, 'devices', fake_devices)
        return fake_devices

    def test_hub_none_skips(self, mod, fake_devices_module):
        """No hub anywhere -> skip cleanly (true on Jetson, Windows dev, clean Linux, etc.)."""
        fake_devices_module.hub = None
        with pytest.raises(pytest.skip.Exception, match="no hub configured"):
            mod.test_hub_recycle_imu_presence(
                (_fake_dev_with_motion(), MagicMock()), 'nightly'
            )

    def test_hub_present_port_unresolved_fails(self, mod, fake_devices_module):
        """Hub detected but device not downstream of it -> port stays None -> fail."""
        fake_devices_module.hub = MagicMock()  # any non-None
        fake_devices_module.get = lambda sn: types.SimpleNamespace(port=None)
        with pytest.raises(pytest.fail.Exception, match="could not resolve a port"):
            mod.test_hub_recycle_imu_presence(
                (_fake_dev_with_motion(), MagicMock()), 'nightly'
            )

    def test_no_motion_module_skips_before_hub_check(self, mod, fake_devices_module):
        """Devices without an IMU should skip with a Motion-Module reason, regardless of hub state."""
        fake_devices_module.hub = None  # would normally trigger the no-hub branch
        sensor = MagicMock()
        sensor.get_info.return_value = "RGB Camera"
        dev = MagicMock()
        dev.query_sensors.return_value = [sensor]
        dev.get_info.return_value = "Intel RealSense D435"
        with pytest.raises(pytest.skip.Exception, match="no Motion Module"):
            mod.test_hub_recycle_imu_presence((dev, MagicMock()), 'nightly')
