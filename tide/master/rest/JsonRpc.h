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

#ifndef JSONRPC_H
#define JSONRPC_H

#include <functional>
#include <future>
#include <memory>
#include <string>

/**
 * JSON-RPC 2.0 request processor.
 *
 * See specification: http://www.jsonrpc.org/specification
 */
class JsonRpc
{
public:
    /**
     * Response to a well-formed RPC request.
     */
    struct Response
    {
        std::string result;
        int error = 0;

        Response() = default;
        Response(std::string&& res)
            : result(res)
        {
        }
        Response(std::string&& res, const int err)
            : result(res)
            , error{err}
        {
        }

        static Response invalidParams() { return {"Invalid params", -32602}; }
    };

    /** @name Asynchronous response to a request. */
    //@{
    using AsyncResponse = std::function<void(Response)>;
    using ProcessAsyncCallback = std::function<void(std::string)>;
    //@}

    /** @name Callbacks that can be registered. */
    //@{
    using ResponseCallback = std::function<Response(const std::string&)>;
    using ResponseCallbackAsync =
        std::function<void(const std::string&, AsyncResponse)>;
    using NotifyCallback = std::function<void(const std::string&)>;
    using VoidCallback = std::function<void()>;
    //@}

    /** Constructor. */
    JsonRpc();

    /** Destructor. */
    ~JsonRpc();

    /**
     * Bind a method to a response callback.
     *
     * @param method to register.
     * @param action to perform.
     * @throw std::invalid_argument if the method name starts with "rpc."
     */
    void bind(const std::string& method, ResponseCallback action);

    /**
     * Bind a method to a response callback with templated request parameters.
     *
     * The parameters object must be deserializable by a free function:
     * from_json(Params& object, const std::string& json).
     *
     * @param method to register.
     * @param action to perform.
     * @throw std::invalid_argument if the method name starts with "rpc."
     */
    template <typename Params>
    void bind(const std::string& method, std::function<Response(Params)> action)
    {
        bind(method, [action](const std::string& request) {
            Params params;
            if (!from_json(params, request))
                return JsonRpc::Response::invalidParams();
            return action(std::move(params));
        });
    }

    /**
     * Bind a method to an async response callback.
     *
     * @param method to register.
     * @param action to perform that will notify the caller upon completion.
     * @throw std::invalid_argument if the method name starts with "rpc."
     */
    void bindAsync(const std::string& method, ResponseCallbackAsync action);

    /**
     * Bind a method to an async response callback with templated parameters.
     *
     * The parameters object must be deserializable by a free function:
     * from_json(Params& object, const std::string& json).
     *
     * @param method to register.
     * @param action to perform that will notify the caller upon completion.
     * @throw std::invalid_argument if the method name starts with "rpc."
     */
    template <typename Params>
    void bindAsync(const std::string& method,
                   std::function<void(Params, AsyncResponse)> action)
    {
        bindAsync(method,
                  [action](const std::string& request, AsyncResponse callback) {
                      Params params;
                      if (from_json(params, request))
                          action(std::move(params), callback);
                      else
                          callback(JsonRpc::Response::invalidParams());
                  });
    }

    /**
     * Bind a method to a callback with request parameters but no response.
     *
     * The templated Params object must be deserializable by a free function:
     * from_json(Params& object, const std::string& json).
     *
     * @param method to register.
     * @param action to perform.
     * @throw std::invalid_argument if the method name starts with "rpc."
     */
    template <typename Params>
    void notify(const std::string& method, std::function<void(Params)> action)
    {
        bind(method, [action](const std::string& request) {
            Params params;
            if (!from_json(params, request))
                return JsonRpc::Response::invalidParams();
            action(std::move(params));
            return JsonRpc::Response{"OK"};
        });
    }

    /**
     * Bind a method to a callback with no response.
     *
     * This is a convienience function that replies with a default "OK" response
     * if the caller asks for one (jsonrpc request id).
     *
     * @param method to register.
     * @param action to perform.
     * @throw std::invalid_argument if the method name starts with "rpc."
     */
    void notify(const std::string& method, NotifyCallback action);

    /**
     * Bind a method to a callback with no response and no payload.
     *
     * This is a convienience function that replies with a default "OK" response
     * if the caller asks for one (jsonrpc request id).
     *
     * @param method to register.
     * @param action to perform.
     * @throw std::invalid_argument if the method name starts with "rpc."
     */
    void notify(const std::string& method, VoidCallback action);

    /**
     * Process a JSON-RPC request and block for the result.
     *
     * @param request string in JSON-RPC 2.0 format.
     * @return json response string in JSON-RPC 2.0 format.
     */
    std::string process(const std::string& request);

    /**
     * Process a JSON-RPC request asynchronously.
     *
     * @param request string in JSON-RPC 2.0 format.
     * @return future json response string in JSON-RPC 2.0 format.
     */
    std::future<std::string> processAsync(const std::string& request);

    /**
     * Process a JSON-RPC request asynchronously.
     *
     * @param request string in JSON-RPC 2.0 format.
     * @param callback that return a json response string in JSON-RPC 2.0
     *        format upon request completion.
     */
    void process(const std::string& request, ProcessAsyncCallback callback);

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

#endif
