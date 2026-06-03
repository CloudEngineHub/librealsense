# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2020 RealSense, Inc. All Rights Reserved.

import time
import threading
from pyrsutils import stabilized_value


# Verify that the stabilized_value class is thread safe.
# We fill the history with the first value and then let 2 threads run.
# First thread adds 66% of first value and the rest of second value.
# Second thread samples the 60% stability value each time and verifies all equal to first value.


#############################################################################################
#
def test_multi_threading():
    inserted_val_1 = 20.0
    inserted_val_2 = 55.0
    values_vec = []
    stab_value = stabilized_value( 10 )

    # Fill history with values (> 60% is 'inserted_val_1')
    def first():
        time.sleep( 0.1 )
        for i in range( 1000 ):
            if i <= 10:
                stab_value.add( inserted_val_1 )
                continue
            if i % 3:
                stab_value.add( inserted_val_1 )
            else:
                stab_value.add( inserted_val_2 )
            time.sleep( 0.001 )

    # Sample stabilized value
    def second():
        while stab_value.empty():
            pass

        for i in range( 1000 ):
            values_vec.append( stab_value.get( 0.6 ) )
            if i % 10:
                time.sleep( 0.001 )

    t1 = threading.Thread( target=first )
    t2 = threading.Thread( target=second )
    t1.start()
    t2.start()
    t1.join()
    t2.join()

    # Verify that all samples show the > 60% number inserted 'inserted_val_1'
    count = sum( 1 for val in values_vec if val == inserted_val_1 )
    assert count == 1000
