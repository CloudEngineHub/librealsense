# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2021 Intel Corporation. All Rights Reserved.

# we want this test to run first so that all tests run with updated FW versions, so we give it priority 0
#test:priority 0
#test:timeout 500
#test:donotrun:gha
#test:device each(D400*)

import sys
import os
import subprocess
import re
import platform
import pyrealsense2 as rs
import pyrsutils as rsutils
from rspy import devices, log, test, file, repo
import time
import argparse

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Test firmware update")
parser.add_argument('--custom-fw-d400', type=str, help='Path to custom firmware file')
args = parser.parse_args()

custom_fw_d400_path = args.custom_fw_d400
if custom_fw_d400_path:
    log.i(f"Custom firmware path provided: {custom_fw_d400_path}")
else:
    log.i(f"No Custom firmware path provided. using bundled firmware")

# This is the first test running, discover acroname modules.
# Not relevant to MIPI devices running on jetson for LibCI
if 'jetson' not in test.context:
    if not devices.hub:
        log.i( "No hub library found; skipping device FW update" )
        sys.exit(0)
    # Following will throw if no acroname module is found
    from rspy import device_hub

    if device_hub.create() is None:
        log.f("No hub found")
    # Remove acroname -- we're likely running inside run-unit-tests in which case the
    # acroname hub is likely already connected-to from there and we'll get an error
    # thrown ('failed to connect to acroname (result=11)'). We do not need it -- just
    # needed to verify it is available above...
    devices.hub = None


def send_hardware_monitor_command(device, command):
    # byte_index = -1
    raw_result = rs.debug_protocol(device).send_and_receive_raw_data(command)

    return raw_result[4:]

import os
import re

def extract_version_from_filename(file_path):
    """
    Extracts the version string from a filename like:
    FlashGeneratedImage_Image5_16_7_0.bin -> 5.16.7
    FlashGeneratedImage_RELEASE_DS5_5_16_3_1.bin -> 5.16.3.1

    Args:
        file_path (str): Full path to the file.

    Returns:
        str: Extracted version in format x.y.z or x.y.z.w, or None if not found or if path is invalid.
    """
    if not file_path or not os.path.exists(file_path):
        log.i(f"File not found: {file_path}")
        return None

    filename = os.path.basename(file_path)
    match = re.search(r'(\d+)_(\d+)_(\d+)_(\d+)\.bin$', filename)
    if match:
        groups = match.groups()
        if groups[3] == '0':
            version_str = ".".join(groups[:3])
        else:
            version_str = ".".join(groups)
        return rsutils.version(version_str)
    else:
        log.i(f"Version not found in filename: {filename}")

    return None


def get_update_counter(device):
    product_line = device.get_info(rs.camera_info.product_line)
    opcode = 0x09
    start_index = 0x30
    size = None

    if product_line == "D400":
        size = 0x2
    else:
        log.f( "Incompatible product line:", product_line )

    raw_cmd = rs.debug_protocol(device).build_command(opcode, start_index, size)
    counter = send_hardware_monitor_command(device, raw_cmd)
    return counter[0]


def reset_update_counter( device ):
    product_line = device.get_info( rs.camera_info.product_line )

    if product_line == "D400":
        opcode = 0x86
        raw_cmd = rs.debug_protocol(device).build_command(opcode)
    else:
        log.f( "Incompatible product line:", product_line )

    send_hardware_monitor_command( device, raw_cmd )

def find_image_or_exit( product_name, fw_version_regex = r'(\d+\.){3}(\d+)' ):
    """
    Searches for a FW image file for the given camera name and optional version. If none are
    found, exits with an error!

    :param product_name: the name of the camera, from get_info(rs.camera_info.name)
    :param fw_version_regex: optional regular expression specifying which FW version image to find

    :return: the image file corresponding to product_name and fw_version if exist, otherwise exit
    """
    pattern = re.compile( r'^Intel RealSense (((\S+?)(\d+))(\S*))' )
    match = pattern.search( product_name )
    if not match:
        raise RuntimeError( "Failed to parse product name '" + product_name + "'" )

    # For a product 'PR567abc', we want to search, in order, these combinations:
    #     PR567abc
    #     PR306abX
    #     PR306aXX
    #     PR306
    #     PR30X
    #     PR3XX
    # Each of the above, combined with the FW version, should yield an image name like:
    #     PR567aXX_FW_Image-<fw-version>.bin
    suffix = 5             # the suffix
    for j in range(1, 3):  # with suffix, then without
        start_index, end_index = match.span(j)
        for i in range(0, len(match.group(suffix))):
            pn = product_name[start_index:end_index-i]
            image_name = '(^|/)' + pn + i*'X' + "_FW_Image-" + fw_version_regex + r'\.bin$'
            for image in file.find(repo.root, image_name):
                return os.path.join( repo.root, image )
        suffix -= 1
    #
    # If we get here, we didn't find any image...
    global product_line
    log.f( "Could not find image file for", product_line )

# find the update tool exe
fw_updater_exe = None
fw_updater_exe_regex = r'(^|/)rs-fw-update'
if platform.system() == 'Windows':
    fw_updater_exe_regex += r'\.exe'
fw_updater_exe_regex += '$'
for tool in file.find( repo.build, fw_updater_exe_regex ):
    fw_updater_exe = os.path.join( repo.build, tool )
if not fw_updater_exe:
    log.f( "Could not find the update tool file (rs-fw-update.exe)" )

devices.query( monitor_changes = False )
sn_list = devices.all()
# acroname should ensure there is always 1 available device
if len( sn_list ) != 1:
    log.f( "Expected 1 device, got", len( sn_list ) )
device = devices.get_first( sn_list ).handle
log.d( 'found:', device )
product_line = device.get_info( rs.camera_info.product_line )
product_name = device.get_info( rs.camera_info.name )
log.d( 'product line:', product_line )
###############################################################################
#


test.start( "Update FW" )
# check if recovery. If so recover
recovered = False
if device.is_update_device():
    log.d( "recovering device ..." )
    try:
        image_file = find_image_or_exit(product_name) if not custom_fw_d400_path else custom_fw_d400_path
        cmd = [fw_updater_exe, '-r', '-f', image_file]
        if custom_fw_d400_path:
            # unsiged fw
            cmd.insert(1, '-u')
        log.d( 'running:', cmd )
        subprocess.run( cmd )
        recovered = True
    except Exception as e:
        test.unexpected_exception()
        log.f( "Unexpected error while trying to recover device:", e )
    else:
        devices.query( monitor_changes = False )
        device = devices.get_first( devices.all() ).handle

current_fw_version = rsutils.version( device.get_info( rs.camera_info.firmware_version ))
log.d( 'current FW version:', current_fw_version )
bundled_fw_version = rsutils.version( device.get_info( rs.camera_info.recommended_firmware_version ) )
log.d( 'bundled FW version:', bundled_fw_version )
custom_fw_d400_version = extract_version_from_filename(custom_fw_d400_path)
log.d( 'custom FW D400 version:', custom_fw_d400_version )


if (current_fw_version == bundled_fw_version and not custom_fw_d400_path) or \
   (current_fw_version == custom_fw_d400_version):
    if recovered or 'nightly' not in test.context:
        log.d('versions are same; skipping FW update')
        test.finish()
        test.print_results_and_exit()
else:
    # It is expected that, post-recovery, the FW versions will be the same
    test.check(not recovered, on_fail=test.ABORT)

update_counter = get_update_counter( device )
log.d( 'update counter:', update_counter )
if update_counter >= 19:
    log.d( 'resetting update counter' )
    reset_update_counter( device )
    update_counter = 0

fw_version_regex = bundled_fw_version.to_string()
if not bundled_fw_version.build():
    fw_version_regex += ".0"  # version drops the build if 0
fw_version_regex = re.escape( fw_version_regex )
image_file = find_image_or_exit(product_name, fw_version_regex) if not custom_fw_d400_path else custom_fw_d400_path
# finding file containing image for FW update

cmd = [fw_updater_exe, '-f', image_file]
if custom_fw_d400_path:
    # unsigned fw
    cmd.insert(1, '-u')
log.d( 'running:', cmd )
sys.stdout.flush()
subprocess.run( cmd )   # may throw

# make sure update worked
time.sleep(3) # MIPI devices do not re-enumerate so we need to give them some time to restart
devices.query( monitor_changes = False )
sn_list = devices.all()
device = devices.get_first( sn_list ).handle
current_fw_version = rsutils.version( device.get_info( rs.camera_info.firmware_version ))
test.check_equal(current_fw_version, bundled_fw_version if not custom_fw_d400_path else custom_fw_d400_version)  
new_update_counter = get_update_counter( device )
# According to FW: "update counter zeros if you load newer FW than (ever) before"
# TODO: check why update counter is 255 when installing cutom fw
if new_update_counter > 0 and not custom_fw_d400_version:
    test.check_equal( new_update_counter, update_counter + 1 )

test.finish()
#
###############################################################################

test.print_results_and_exit()
