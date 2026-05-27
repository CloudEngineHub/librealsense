# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2026 RealSense, Inc. All Rights Reserved.

from pydantic import BaseModel, Field
from typing import List


class HwmRequest(BaseModel):
    """Request body for a hardware monitor (HWM) command."""

    opcode: int = Field(..., description="HWM opcode (e.g. 0x10 for GVD)")
    param1: int = Field(0, description="First command parameter (default 0)")
    param2: int = Field(0, description="Second command parameter (default 0)")
    param3: int = Field(0, description="Third command parameter (default 0)")
    param4: int = Field(0, description="Fourth command parameter (default 0)")
    data: List[int] = Field(
        default_factory=list,
        description="Optional payload bytes appended after the header",
    )


class HwmResponse(BaseModel):
    """Response returned by a hardware monitor command."""

    device_id: str = Field(..., description="Serial number of the target device")
    response: List[int] = Field(
        ..., description="Raw firmware response as a list of byte values"
    )
