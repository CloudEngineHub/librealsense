// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2025 Intel Corporation. All Rights Reserved.

/*!
 * @file SetParameters.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_RCL_INTERFACES_SRV_SETPARAMETERS_H_
#define _FAST_DDS_GENERATED_RCL_INTERFACES_SRV_SETPARAMETERS_H_

#include "../msg/Parameter.h"
#include "../msg/SetParametersResult.h"

//#include <fastrtps/utils/fixed_size_string.hpp>

#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <bitset>

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(SetParameters_SOURCE)
#define SetParameters_DllAPI __declspec( dllexport )
#else
#define SetParameters_DllAPI __declspec( dllimport )
#endif // SetParameters_SOURCE
#else
#define SetParameters_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define SetParameters_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima


namespace rcl_interfaces {
    namespace srv {
        /*!
         * @brief This class represents the structure SetParameters_Request defined by the user in the IDL file.
         * @ingroup SETPARAMETERS
         */
        class SetParameters_Request
        {
        public:

            /*!
             * @brief Default constructor.
             */
            eProsima_user_DllExport SetParameters_Request();

            /*!
             * @brief Default destructor.
             */
            eProsima_user_DllExport ~SetParameters_Request();

            /*!
             * @brief Copy constructor.
             * @param x Reference to the object rcl_interfaces::srv::SetParameters_Request that will be copied.
             */
            eProsima_user_DllExport SetParameters_Request(
                    const SetParameters_Request& x);

            /*!
             * @brief Move constructor.
             * @param x Reference to the object rcl_interfaces::srv::SetParameters_Request that will be copied.
             */
            eProsima_user_DllExport SetParameters_Request(
                    SetParameters_Request&& x) noexcept;

            /*!
             * @brief Copy assignment.
             * @param x Reference to the object rcl_interfaces::srv::SetParameters_Request that will be copied.
             */
            eProsima_user_DllExport SetParameters_Request& operator =(
                    const SetParameters_Request& x);

            /*!
             * @brief Move assignment.
             * @param x Reference to the object rcl_interfaces::srv::SetParameters_Request that will be copied.
             */
            eProsima_user_DllExport SetParameters_Request& operator =(
                    SetParameters_Request&& x) noexcept;

            /*!
             * @brief Comparison operator.
             * @param x rcl_interfaces::srv::SetParameters_Request object to compare.
             */
            eProsima_user_DllExport bool operator ==(
                    const SetParameters_Request& x) const;

            /*!
             * @brief Comparison operator.
             * @param x rcl_interfaces::srv::SetParameters_Request object to compare.
             */
            eProsima_user_DllExport bool operator !=(
                    const SetParameters_Request& x) const;

            /*!
             * @brief This function copies the value in member parameters
             * @param _parameters New value to be copied in member parameters
             */
            eProsima_user_DllExport void parameters(
                    const std::vector<rcl_interfaces::msg::Parameter>& _parameters);

            /*!
             * @brief This function moves the value in member parameters
             * @param _parameters New value to be moved in member parameters
             */
            eProsima_user_DllExport void parameters(
                    std::vector<rcl_interfaces::msg::Parameter>&& _parameters);

            /*!
             * @brief This function returns a constant reference to member parameters
             * @return Constant reference to member parameters
             */
            eProsima_user_DllExport const std::vector<rcl_interfaces::msg::Parameter>& parameters() const;

            /*!
             * @brief This function returns a reference to member parameters
             * @return Reference to member parameters
             */
            eProsima_user_DllExport std::vector<rcl_interfaces::msg::Parameter>& parameters();

            /*!
             * @brief This function returns the maximum serialized size of an object
             * depending on the buffer alignment.
             * @param current_alignment Buffer alignment.
             * @return Maximum serialized size.
             */
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!
             * @brief This function returns the serialized size of a data depending on the buffer alignment.
             * @param data Data which is calculated its serialized size.
             * @param current_alignment Buffer alignment.
             * @return Serialized size.
             */
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const rcl_interfaces::srv::SetParameters_Request& data,
                    size_t current_alignment = 0);


            /*!
             * @brief This function serializes an object using CDR serialization.
             * @param cdr CDR serialization object.
             */
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            /*!
             * @brief This function deserializes an object using CDR serialization.
             * @param cdr CDR serialization object.
             */
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);



            /*!
             * @brief This function returns the maximum serialized size of the Key of an object
             * depending on the buffer alignment.
             * @param current_alignment Buffer alignment.
             * @return Maximum serialized size.
             */
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!
             * @brief This function tells you if the Key has been defined for this type
             */
            eProsima_user_DllExport static bool isKeyDefined();

            /*!
             * @brief This function serializes the key members of an object using CDR serialization.
             * @param cdr CDR serialization object.
             */
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:

            std::vector<rcl_interfaces::msg::Parameter> m_parameters;
        };
        /*!
         * @brief This class represents the structure SetParameters_Response defined by the user in the IDL file.
         * @ingroup SETPARAMETERS
         */
        class SetParameters_Response
        {
        public:

            /*!
             * @brief Default constructor.
             */
            eProsima_user_DllExport SetParameters_Response();

            /*!
             * @brief Default destructor.
             */
            eProsima_user_DllExport ~SetParameters_Response();

            /*!
             * @brief Copy constructor.
             * @param x Reference to the object rcl_interfaces::srv::SetParameters_Response that will be copied.
             */
            eProsima_user_DllExport SetParameters_Response(
                    const SetParameters_Response& x);

            /*!
             * @brief Move constructor.
             * @param x Reference to the object rcl_interfaces::srv::SetParameters_Response that will be copied.
             */
            eProsima_user_DllExport SetParameters_Response(
                    SetParameters_Response&& x) noexcept;

            /*!
             * @brief Copy assignment.
             * @param x Reference to the object rcl_interfaces::srv::SetParameters_Response that will be copied.
             */
            eProsima_user_DllExport SetParameters_Response& operator =(
                    const SetParameters_Response& x);

            /*!
             * @brief Move assignment.
             * @param x Reference to the object rcl_interfaces::srv::SetParameters_Response that will be copied.
             */
            eProsima_user_DllExport SetParameters_Response& operator =(
                    SetParameters_Response&& x) noexcept;

            /*!
             * @brief Comparison operator.
             * @param x rcl_interfaces::srv::SetParameters_Response object to compare.
             */
            eProsima_user_DllExport bool operator ==(
                    const SetParameters_Response& x) const;

            /*!
             * @brief Comparison operator.
             * @param x rcl_interfaces::srv::SetParameters_Response object to compare.
             */
            eProsima_user_DllExport bool operator !=(
                    const SetParameters_Response& x) const;

            /*!
             * @brief This function copies the value in member results
             * @param _results New value to be copied in member results
             */
            eProsima_user_DllExport void results(
                    const std::vector<rcl_interfaces::msg::SetParametersResult>& _results);

            /*!
             * @brief This function moves the value in member results
             * @param _results New value to be moved in member results
             */
            eProsima_user_DllExport void results(
                    std::vector<rcl_interfaces::msg::SetParametersResult>&& _results);

            /*!
             * @brief This function returns a constant reference to member results
             * @return Constant reference to member results
             */
            eProsima_user_DllExport const std::vector<rcl_interfaces::msg::SetParametersResult>& results() const;

            /*!
             * @brief This function returns a reference to member results
             * @return Reference to member results
             */
            eProsima_user_DllExport std::vector<rcl_interfaces::msg::SetParametersResult>& results();

            /*!
             * @brief This function returns the maximum serialized size of an object
             * depending on the buffer alignment.
             * @param current_alignment Buffer alignment.
             * @return Maximum serialized size.
             */
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!
             * @brief This function returns the serialized size of a data depending on the buffer alignment.
             * @param data Data which is calculated its serialized size.
             * @param current_alignment Buffer alignment.
             * @return Serialized size.
             */
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const rcl_interfaces::srv::SetParameters_Response& data,
                    size_t current_alignment = 0);


            /*!
             * @brief This function serializes an object using CDR serialization.
             * @param cdr CDR serialization object.
             */
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            /*!
             * @brief This function deserializes an object using CDR serialization.
             * @param cdr CDR serialization object.
             */
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);



            /*!
             * @brief This function returns the maximum serialized size of the Key of an object
             * depending on the buffer alignment.
             * @param current_alignment Buffer alignment.
             * @return Maximum serialized size.
             */
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!
             * @brief This function tells you if the Key has been defined for this type
             */
            eProsima_user_DllExport static bool isKeyDefined();

            /*!
             * @brief This function serializes the key members of an object using CDR serialization.
             * @param cdr CDR serialization object.
             */
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:

            std::vector<rcl_interfaces::msg::SetParametersResult> m_results;
        };
    } // namespace srv
} // namespace rcl_interfaces

#endif // _FAST_DDS_GENERATED_RCL_INTERFACES_SRV_SETPARAMETERS_H_
