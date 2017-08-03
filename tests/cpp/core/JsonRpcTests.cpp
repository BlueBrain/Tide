/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#define BOOST_TEST_MODULE JsonRpcTests

#include <boost/test/unit_test.hpp>

#include "rest/JsonRpc.h"
#include "rest/json.h"

#include <thread>

// Validation examples based on: http://www.jsonrpc.org/specification

namespace
{
const std::string substractArray{
    R"({"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": 3})"};

const std::string substractArrayStringId{
    R"({"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": "myId123"})"};

const std::string substractObject{
    R"({"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 3})"};

const std::string substractNotification{
    R"({"jsonrpc": "2.0", "method": "subtract", "params": [42, 23]})"};

const std::string substractBatch{
    R"([{"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 1},
        {"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 3}])"};

const std::string substractBatchNotification{
    R"([{"jsonrpc": "2.0", "method": "subtract", "params": [42, 23]},
        {"jsonrpc": "2.0", "method": "subtract", "params": [42, 23]}])"};

const std::string substractBatchMixed{
    R"([{"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": 1},
        {"jsonrpc": "2.0", "method": "subtract", "params": [42, 23]},
        {"jsonrpc": "2.0", "method": "subtract", "params": [42, 23], "id": 3},
        {"jsonrpc": "2.0", "method": "subtract", "params": [42, 23]}])"};

const std::string invalidRequest{
    R"({"jsonrpc": "2.0", "method": 1, "params": "bar"})"};

const std::string invalidJsonRpcVersionRequest{
    R"({"jsonrpc": "3.5", "method": "subtract", "params": [42, 23], "id": 3})"};

const std::string substractResult{
    R"({
    "id": 3,
    "jsonrpc": "2.0",
    "result": "19"
}
)"};

const std::string substractResultStringId{
    R"({
    "id": "myId123",
    "jsonrpc": "2.0",
    "result": "19"
}
)"};

const std::string notifyStandardReply{
    R"({
    "id": 3,
    "jsonrpc": "2.0",
    "result": "OK"
}
)"};

const std::string substractBatchResult{
    R"([
    {
        "id": 1,
        "jsonrpc": "2.0",
        "result": "19"
    },
    {
        "id": 3,
        "jsonrpc": "2.0",
        "result": "19"
    }
]
)"};

const std::string nonExistantMethodResult{
    R"({
    "error": {
        "code": -32601,
        "message": "Method not found"
    },
    "id": 3,
    "jsonrpc": "2.0"
}
)"};

const std::string invalidJsonResult{
    R"({
    "error": {
        "code": -32700,
        "message": "Parse error"
    },
    "id": null,
    "jsonrpc": "2.0"
}
)"};

const std::string invalidRequestResult{
    R"({
    "error": {
        "code": -32600,
        "message": "Invalid Request"
    },
    "id": null,
    "jsonrpc": "2.0"
}
)"};

const std::string invalidParamsResult{
    R"({
    "error": {
        "code": -32602,
        "message": "Invalid params"
    },
    "id": 3,
    "jsonrpc": "2.0"
}
)"};

const std::string invalidBatch1RequestResult{
    R"([
    {
        "error": {
            "code": -32600,
            "message": "Invalid Request"
        },
        "id": null,
        "jsonrpc": "2.0"
    }
]
)"};

const std::string invalidBatch3RequestResult{
    R"([
    {
        "error": {
            "code": -32600,
            "message": "Invalid Request"
        },
        "id": null,
        "jsonrpc": "2.0"
    },
    {
        "error": {
            "code": -32600,
            "message": "Invalid Request"
        },
        "id": null,
        "jsonrpc": "2.0"
    },
    {
        "error": {
            "code": -32600,
            "message": "Invalid Request"
        },
        "id": null,
        "jsonrpc": "2.0"
    }
]
)"};
}

JsonRpc::Response substractObj(const std::string& params)
{
    const auto object = json::toObject(params);

    if (!object["minuend"].isDouble() || !object["subtrahend"].isDouble())
        return JsonRpc::Response::invalidParams();

    const auto value = object["minuend"].toInt() - object["subtrahend"].toInt();
    return {std::to_string(value)};
}

JsonRpc::Response substractArr(const std::string& params)
{
    const auto array = json::toArray(params);

    if (array.size() != 2 || !array.at(0).isDouble() || !array.at(1).isDouble())
        return JsonRpc::Response::invalidParams();

    const auto value = array[0].toInt() - array[1].toInt();
    return {std::to_string(value)};
}

void substractArrAsync(const std::string& params,
                       JsonRpc::AsyncResponse callback)
{
    std::thread([params, callback]() {
        usleep(500);
        callback(substractArr(params));
    }).detach();
}

struct Operands
{
    int left = 0;
    int right = 0;
};
bool from_json(Operands& op, const std::string& json)
{
    const auto object = json::toObject(json);
    if (!object["minuend"].isDouble() || !object["subtrahend"].isDouble())
        return false;
    op.left = object["minuend"].toInt();
    op.right = object["subtrahend"].toInt();
    return true;
}

struct Fixture
{
    JsonRpc jsonRpc;
};

BOOST_FIXTURE_TEST_CASE(process_obj, Fixture)
{
    jsonRpc.bind("subtract", std::bind(&substractObj, std::placeholders::_1));
    BOOST_CHECK_EQUAL(jsonRpc.process(substractObject), substractResult);
}

BOOST_FIXTURE_TEST_CASE(process_arr, Fixture)
{
    jsonRpc.bind("subtract", std::bind(&substractArr, std::placeholders::_1));
    BOOST_CHECK_EQUAL(jsonRpc.process(substractArray), substractResult);
}

BOOST_FIXTURE_TEST_CASE(process_arr_async, Fixture)
{
    using namespace std::placeholders;
    jsonRpc.bindAsync("subtract", std::bind(&substractArrAsync, _1, _2));
    BOOST_CHECK_EQUAL(jsonRpc.process(substractArray), substractResult);
}

BOOST_FIXTURE_TEST_CASE(process_arr_with_string_id, Fixture)
{
    jsonRpc.bind("subtract", std::bind(&substractArr, std::placeholders::_1));
    BOOST_CHECK_EQUAL(jsonRpc.process(substractArrayStringId),
                      substractResultStringId);
}

BOOST_FIXTURE_TEST_CASE(process_notification, Fixture)
{
    // Notifications are rpc requests without an id.
    // They must be processed without returning a response.
    bool called = false;
    JsonRpc::Response response;
    JsonRpc::ResponseCallback action = [&called,
                                        &response](const std::string& params) {
        called = true;
        response = substractArr(params);
        return response;
    };
    jsonRpc.bind("subtract", action);
    BOOST_CHECK(jsonRpc.process(substractNotification).empty());
    BOOST_CHECK(called);
    BOOST_CHECK_EQUAL(response.result, "19");
}

BOOST_FIXTURE_TEST_CASE(bind_with_params, Fixture)
{
    jsonRpc.bind<Operands>("subtract", [](const Operands op) {
        return JsonRpc::Response{std::to_string(op.left - op.right)};
    });
    BOOST_CHECK_EQUAL(jsonRpc.process(substractObject), substractResult);
    BOOST_CHECK_EQUAL(jsonRpc.process(substractArray), invalidParamsResult);
}

BOOST_FIXTURE_TEST_CASE(bind_async_with_params, Fixture)
{
    jsonRpc.bindAsync<Operands>(
        "subtract", [](const Operands op, JsonRpc::AsyncResponse callback) {
            callback(JsonRpc::Response{std::to_string(op.left - op.right)});
        });
    BOOST_CHECK_EQUAL(jsonRpc.process(substractObject), substractResult);
    BOOST_CHECK_EQUAL(jsonRpc.process(substractArray), invalidParamsResult);
}

BOOST_FIXTURE_TEST_CASE(notify_with_params, Fixture)
{
    jsonRpc.notify<Operands>("subtract", [](const Operands op) {
        return op.left == 42 && op.right == 23;
    });
    BOOST_CHECK_EQUAL(jsonRpc.process(substractObject), notifyStandardReply);
    BOOST_CHECK_EQUAL(jsonRpc.process(substractArray), invalidParamsResult);
}

BOOST_FIXTURE_TEST_CASE(reserved_method_names, Fixture)
{
    using namespace std::placeholders;
    const auto bindFunc = std::bind(&substractObj, _1);
    const auto bindAsyncFunc = std::bind(&substractArrAsync, _1, _2);

    BOOST_CHECK_THROW(jsonRpc.bind("rpc.xyz", bindFunc), std::invalid_argument);
    BOOST_CHECK_THROW(jsonRpc.bindAsync("rpc.abc", bindAsyncFunc),
                      std::invalid_argument);
    BOOST_CHECK_THROW(jsonRpc.notify("rpc.", [](const std::string&) {}),
                      std::invalid_argument);
    BOOST_CHECK_THROW(jsonRpc.notify("rpc.void", [] {}), std::invalid_argument);

    BOOST_CHECK_NO_THROW(jsonRpc.bind("RPC.xyz", bindFunc));
    BOOST_CHECK_NO_THROW(jsonRpc.bind("rpc", bindFunc));
    BOOST_CHECK_NO_THROW(jsonRpc.bind("_rpc.", bindFunc));
}

BOOST_FIXTURE_TEST_CASE(non_existant_method, Fixture)
{
    BOOST_CHECK_EQUAL(jsonRpc.process(substractArray), nonExistantMethodResult);
}

BOOST_FIXTURE_TEST_CASE(invalid_json, Fixture)
{
    jsonRpc.bind("subtract", std::bind(&substractObj, std::placeholders::_1));
    BOOST_CHECK_EQUAL(jsonRpc.process("Zorgy!!"), invalidJsonResult);
}

BOOST_FIXTURE_TEST_CASE(invalid_json_rpc_request, Fixture)
{
    jsonRpc.bind("subtract", std::bind(&substractObj, std::placeholders::_1));
    BOOST_CHECK_EQUAL(jsonRpc.process(invalidJsonRpcVersionRequest),
                      invalidRequestResult);
}

BOOST_FIXTURE_TEST_CASE(wrong_json_rpc_version, Fixture)
{
    jsonRpc.bind("subtract", std::bind(&substractObj, std::placeholders::_1));
    BOOST_CHECK_EQUAL(jsonRpc.process(invalidRequest), invalidRequestResult);
}

BOOST_FIXTURE_TEST_CASE(invalid_array_requests, Fixture)
{
    jsonRpc.bind("subtract", std::bind(&substractObj, std::placeholders::_1));
    BOOST_CHECK_EQUAL(jsonRpc.process("[\"Zorgy!\": 1]"), invalidJsonResult);

    BOOST_CHECK_EQUAL(jsonRpc.process("[]"), invalidRequestResult);
    BOOST_CHECK_EQUAL(jsonRpc.process("[1]"), invalidBatch1RequestResult);
    BOOST_CHECK_EQUAL(jsonRpc.process("[1,2,3]"), invalidBatch3RequestResult);
}

BOOST_FIXTURE_TEST_CASE(valid_array_request, Fixture)
{
    jsonRpc.bind("subtract", std::bind(&substractObj, std::placeholders::_1));
    BOOST_CHECK_EQUAL(jsonRpc.process(substractBatch), substractBatchResult);
}

BOOST_FIXTURE_TEST_CASE(process_array_notification, Fixture)
{
    // Notifications are rpc requests without an id.
    // A batch of notification must be processed without returning a response.
    int called = 0;
    JsonRpc::Response response;
    JsonRpc::ResponseCallback action = [&called,
                                        &response](const std::string& params) {
        ++called;
        response = substractArr(params);
        return response;
    };
    jsonRpc.bind("subtract", action);
    BOOST_CHECK(jsonRpc.process(substractBatchNotification).empty());
    BOOST_CHECK_EQUAL(called, 2);
    BOOST_CHECK_EQUAL(response.result, "19");
}

BOOST_FIXTURE_TEST_CASE(process_array_mixed, Fixture)
{
    // For mixed notifications/requests batches, only respond to requests.
    int called = 0;
    JsonRpc::Response response;
    auto action = [&called, &response](const std::string& params) {
        ++called;
        response = substractArr(params);
        return response;
    };
    jsonRpc.bind("subtract", JsonRpc::ResponseCallback(action));
    BOOST_CHECK_EQUAL(jsonRpc.process(substractBatchMixed),
                      substractBatchResult);
    BOOST_CHECK_EQUAL(called, 4);
    BOOST_CHECK_EQUAL(response.result, "19");
}
