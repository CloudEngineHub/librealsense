# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2022 RealSense, Inc. All Rights Reserved.

import re
import pytest
from pytest_check import check
from pyrsutils import stabilized_value


#############################################################################################
#
def test_get_60_percent_stability():
    #
    # Verify if history is filled with a stable value at the required percentage, the user
    # will get it when asked for even if other inputs exist in history
    #
    sv = stabilized_value( 10 )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    check.equal( sv.get( 0.6 ), 55. )


#############################################################################################
#
def test_change_stabilization_percent():
    #
    # Verify that on the same values different 'get' calls with different stabilization percent
    # give the expected value
    #
    sv = stabilized_value( 5 )
    sv.add( 1. )
    sv.add( 1. )
    sv.add( 1. )
    sv.add( 1. )
    sv.add( 1. )

    sv.add( 2. )
    sv.add( 2. )
    sv.add( 2. )
    sv.add( 2. )

    check.equal( sv.get( 1.  ), 1. )
    check.equal( sv.get( 0.9 ), 1. )
    check.equal( sv.get( 0.8 ), 2. )


#############################################################################################
#
def test_empty():
    #
    # Verify that empty function works as expected
    #
    sv = stabilized_value( 5 )
    check.is_true( sv.empty() )
    check.is_false( sv )
    sv.add( 1. )
    check.is_false( sv.empty() )
    sv.clear()
    check.is_true( sv.empty() )


#############################################################################################
#
def test_update_stable_value():
    #
    # Verify a stabilized value once the inserted value is flickering between 2 values
    #
    sv = stabilized_value( 10 )
    # Verify flickering value always report the stable value
    for i in range(100):
        sv.add( 55. )
        sv.add( 60. )
        check.equal( sv.get( 0.7 ), 55. )


#############################################################################################
#
def test_illegal_input_percentage_too_high():
    #
    # Verify stabilized_value percentage input is at range (0-100] % (zero not included)
    #
    sv = stabilized_value( 5 )
    sv.add( 55. )
    with pytest.raises( RuntimeError, match=re.escape( "illegal stabilization percentage 1.100000" ) ):
        sv.get( 1.1 )
    with pytest.raises( RuntimeError, match=re.escape( "illegal stabilization percentage -1.100000" ) ):
        sv.get( -1.1 )
    with pytest.raises( RuntimeError, match=re.escape( "illegal stabilization percentage 0.000000" ) ):
        sv.get( 0.0 )


#############################################################################################
#
def test_not_full_history():
    #
    # Verify if history is not the logic works as expected and the percentage is calculated
    # from the history current size
    #
    sv = stabilized_value( 30 )
    sv.add( 76. )
    sv.add( 76. )
    sv.add( 76. )
    sv.add( 76. )
    check.equal( sv.get( 0.6 ), 76. )

    sv.add( 45. )
    sv.add( 45. )
    sv.add( 45. )
    sv.add( 45. )
    sv.add( 45. )

    check.equal( sv.get( 0.6 ), 76. )
    sv.add( 45. )  # The stable value should change now (4 * 76.0 + 6 * 45.0 (total 10 values))
    check.equal( sv.get( 0.6 ), 45. )


#############################################################################################
#
def test_stable_value_sanity():
    #
    # Verify if history is full with a specific value, the stabilized value is always the same
    # no matter what percentage is required
    #
    sv = stabilized_value( 5 )
    sv.add( 1. )
    sv.add( 1. )
    sv.add( 1. )
    sv.add( 1. )
    sv.add( 1. )

    check.equal( sv.get( 1.   ), 1. )
    check.equal( sv.get( 0.4  ), 1. )
    check.equal( sv.get( 0.25 ), 1. )


#############################################################################################
#
def test_stay_with_last_value():
    #
    # Verify if history is filled with less stable percantage than required the last stable
    # value is returned
    #
    sv = stabilized_value( 10 )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )

    check.equal( sv.get( 0.6 ), 55. )


#############################################################################################
#
# Verify if history is filled with a stable value and then filled with required percentage
# of new val, new val is returned as stable value.
#
def test_update_stable_value_nominal():
    sv = stabilized_value( 10 )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 55. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )

    check.equal( sv.get( 0.6 ), 60. )
    sv.add( 35. )
    sv.add( 35. )
    sv.add( 35. )
    sv.add( 35. )
    sv.add( 35. )
    check.equal( sv.get( 0.6 ), 60. )

    sv.add( 35. )
    check.equal( sv.get( 0.6 ), 35. )


def test_update_stable_value_last_stable_not_in_history():
    sv = stabilized_value( 10 )
    sv.add( 55. )
    check.equal( sv.get( 1. ), 55. )

    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    sv.add( 60. )
    check.equal( sv.get( 1.  ), 55. )
    check.equal( sv.get( 0.9 ), 60. )
    sv.add( 70. )
    check.equal( sv.get( 1.  ), 60. )


def test_update_stable_value_last_stable_is_in_history():
    sv = stabilized_value( 10 )
    sv.add( 55. )
    sv.add( 60. )
    sv.add( 60. )
    check.equal( sv.get( 0.8 ), 55. )
