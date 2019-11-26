// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

/**
 * \file
 * Common multicast combinator classes.
 */

#include "config.hpp"
#include "mixin_collection.hpp"

namespace dynamix
{
namespace combinators
{

/**
 * A combinator for a boolean `and` operation.
 *
 * \tparam MessageReturnType The actual return type of the messages.
 * Its default value is `bool`, but such an operation is valid for
 * various other classes that can be cast to `bool`, such as pointers,
 * std streams, etc.
 */
template <typename MessageReturnType = bool>
class boolean_and
{
public:
    typedef bool result_type;

    boolean_and()
        : _result(true)
    {}

    /// The function used by the code generated for multicast messages.
    /// Stops the execution of the multicast chain at the first `false`.
    bool add_result(const MessageReturnType& r)
    {
        _result = _result && bool(r);

        return _result; // stop at the first false
    }

    /// The result of the operation if the multicast call has been made
    /// with an input/ouput parameter - an instance of `boolean_and`
    bool result() const
    {
        return _result;
    }

    /// Resets the result, so the instance could be reused.
    void reset()
    {
        _result = true;
    }

private:
    bool _result;
};

/**
 * A combinator for a boolean `or` operation.
 *
 * \tparam MessageReturnType The actual return type of the messages.
 * Its default value is `bool`, but such an operation is valid for
 * various other classes that can be cast to `bool`, such as pointers,
 * std streams, etc.
 */
template <typename MessageReturnType = bool>
class boolean_or
{
public:
    typedef bool result_type;

    boolean_or()
        : _result(false)
    {}

    /// The function used by the code generated for multicast messages.
    /// Stops the execution of the multicast chain at the first `true`.
    bool add_result(const MessageReturnType& r)
    {
        _result = _result || bool(r);

        return !_result; // stop at the first true
    }

    /// The result of the operation if the multicast call has been made
    /// with an input/output parameter - an instance of `boolean_or`
    bool result() const
    {
        return _result;
    }

    /// Resets the result, so the instance could be reused.
    void reset()
    {
        _result = false;
    }

private:
    bool _result;
};

/**
 * A combinator for a sum operation. Sums all the return values of
 * the multicast chain.
 *
 * \tparam MessageReturnType The actual return type of the messages.
 */
template <typename MessageReturnType>
class sum
{
public:
    typedef MessageReturnType result_type;

    sum()
        : _result(0)
    {}

    /// The function used by the code generated for multicast messages.
    bool add_result(const MessageReturnType& r)
    {
        _result += r;
        return true;
    }

    /// The result of the operation if the multicast call has been made
    /// with an input/output parameter - an instance of `sum`
    const result_type& result() const
    {
        return _result;
    }

    /// Resets the result, so the instance could be reused.
    void reset()
    {
        _result = 0;
    }

private:
    result_type _result;
};

/**
* A combinator that calculates an arithmetic mean of all return values in the
* multicast chain
*
* \tparam MessageReturnType The actual return type of the messages.
*/
template <typename MessageReturnType>
class mean
{
public:
    typedef MessageReturnType result_type;

    mean()
        : _sum(0)
        , _num_results(0)
    {}

    /// The function called by the multicast caller to set the number of results
    void set_num_results(size_t num)
    {
        _num_results = num;
    }

    /// The function used by the code generated for multicast messages.
    bool add_result(const MessageReturnType& r)
    {
        _sum += r;
        return true;
    }

    /// The result of the operation if the multicast call has been made
    /// with an input/output parameter - an instance of `mean`
    result_type result() const
    {
        return _sum / result_type(_num_results);
    }

    /// Resets the result, so the instance could be reused.
    void reset()
    {
        _sum = 0;
    }

private:
    result_type _sum;
    size_t _num_results;
};

} // namespace combinators
} // namespace dynamix
