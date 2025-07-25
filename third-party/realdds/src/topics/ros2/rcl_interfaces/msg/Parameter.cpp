// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2025 Intel Corporation. All Rights Reserved.

/*!
 * @file Parameter.cpp
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace {
char dummy;
}  // namespace
#endif  // _WIN32

#include <realdds/topics/ros2/rcl_interfaces/msg/Parameter.h>
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

rcl_interfaces::msg::Parameter::Parameter()
{
    // m_name com.eprosima.idl.parser.typecode.StringTypeCode@19dc67c2
    m_name ="";
    // m_value com.eprosima.fastdds.idl.parser.typecode.StructTypeCode@62bd765


}

rcl_interfaces::msg::Parameter::~Parameter()
{


}

rcl_interfaces::msg::Parameter::Parameter(
        const Parameter& x)
{
    m_name = x.m_name;
    m_value = x.m_value;
}

rcl_interfaces::msg::Parameter::Parameter(
        Parameter&& x) noexcept 
{
    m_name = std::move(x.m_name);
    m_value = std::move(x.m_value);
}

rcl_interfaces::msg::Parameter& rcl_interfaces::msg::Parameter::operator =(
        const Parameter& x)
{

    m_name = x.m_name;
    m_value = x.m_value;

    return *this;
}

rcl_interfaces::msg::Parameter& rcl_interfaces::msg::Parameter::operator =(
        Parameter&& x) noexcept
{

    m_name = std::move(x.m_name);
    m_value = std::move(x.m_value);

    return *this;
}

bool rcl_interfaces::msg::Parameter::operator ==(
        const Parameter& x) const
{

    return (m_name == x.m_name && m_value == x.m_value);
}

bool rcl_interfaces::msg::Parameter::operator !=(
        const Parameter& x) const
{
    return !(*this == x);
}

size_t rcl_interfaces::msg::Parameter::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 255 + 1;

    current_alignment += rcl_interfaces::msg::ParameterValue::getMaxCdrSerializedSize(current_alignment);

    return current_alignment - initial_alignment;
}

size_t rcl_interfaces::msg::Parameter::getCdrSerializedSize(
        const rcl_interfaces::msg::Parameter& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.name().size() + 1;

    current_alignment += rcl_interfaces::msg::ParameterValue::getCdrSerializedSize(data.value(), current_alignment);

    return current_alignment - initial_alignment;
}

void rcl_interfaces::msg::Parameter::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{

    scdr << m_name.c_str();
    scdr << m_value;

}

void rcl_interfaces::msg::Parameter::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{

    dcdr >> m_name;
    dcdr >> m_value;
}

/*!
 * @brief This function copies the value in member name
 * @param _name New value to be copied in member name
 */
void rcl_interfaces::msg::Parameter::name(
        const std::string& _name)
{
    m_name = _name;
}

/*!
 * @brief This function moves the value in member name
 * @param _name New value to be moved in member name
 */
void rcl_interfaces::msg::Parameter::name(
        std::string&& _name)
{
    m_name = std::move(_name);
}

/*!
 * @brief This function returns a constant reference to member name
 * @return Constant reference to member name
 */
const std::string& rcl_interfaces::msg::Parameter::name() const
{
    return m_name;
}

/*!
 * @brief This function returns a reference to member name
 * @return Reference to member name
 */
std::string& rcl_interfaces::msg::Parameter::name()
{
    return m_name;
}
/*!
 * @brief This function copies the value in member value
 * @param _value New value to be copied in member value
 */
void rcl_interfaces::msg::Parameter::value(
        const rcl_interfaces::msg::ParameterValue& _value)
{
    m_value = _value;
}

/*!
 * @brief This function moves the value in member value
 * @param _value New value to be moved in member value
 */
void rcl_interfaces::msg::Parameter::value(
        rcl_interfaces::msg::ParameterValue&& _value)
{
    m_value = std::move(_value);
}

/*!
 * @brief This function returns a constant reference to member value
 * @return Constant reference to member value
 */
const rcl_interfaces::msg::ParameterValue& rcl_interfaces::msg::Parameter::value() const
{
    return m_value;
}

/*!
 * @brief This function returns a reference to member value
 * @return Reference to member value
 */
rcl_interfaces::msg::ParameterValue& rcl_interfaces::msg::Parameter::value()
{
    return m_value;
}

size_t rcl_interfaces::msg::Parameter::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t current_align = current_alignment;





    return current_align;
}

bool rcl_interfaces::msg::Parameter::isKeyDefined()
{
    return false;
}

void rcl_interfaces::msg::Parameter::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
      
}


