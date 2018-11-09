/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#ifndef ITERABLE_SMART_PTR_COLLECTION_H
#define ITERABLE_SMART_PTR_COLLECTION_H

#include <iterator>
#include <type_traits>

/**
 * Facade class that lets users safely iterate over a collection of smart_ptrs.
 *
 * Iterating a collection through this class gives direct access to the pointed
 * objects, helping:
 * a) Enforce const correctness: the pointed objects can't be mutated when
 *    iterating over a const container<smart_ptr>.
 * b) Improve readability: no need to dereference the pointer in user code.
 */
template <typename ContainerT>
class IterableSmartPtrCollection
{
public:
    template <bool is_const_iterator = true>
    struct smart_ptr_iterator_wrapper
    {
        // std::iterator_traits
        using difference_type = typename ContainerT::iterator::difference_type;
        using value_type = typename ContainerT::value_type::element_type;
        using reference =
            typename std::conditional_t<is_const_iterator, const value_type&,
                                        value_type&>;
        using pointer =
            typename std::conditional_t<is_const_iterator, const value_type*,
                                        value_type*>;
        using iterator_category = std::forward_iterator_tag;

        // internal
        using smart_ptr_iterator =
            typename std::conditional_t<is_const_iterator,
                                        typename ContainerT::const_iterator,
                                        typename ContainerT::iterator>;

        smart_ptr_iterator_wrapper(smart_ptr_iterator it)
            : _it{it}
        {
        }
        smart_ptr_iterator_wrapper operator++()
        {
            return smart_ptr_iterator_wrapper(++_it);
        }
        bool operator!=(smart_ptr_iterator_wrapper other) const
        {
            return _it != other._it;
        }
        reference operator*() { return *(*_it); }
        pointer operator->() const { return _it->get(); }
    private:
        smart_ptr_iterator _it;
    };

    using iterator = smart_ptr_iterator_wrapper<false>;
    using const_iterator = smart_ptr_iterator_wrapper<true>;
    using value_type = typename ContainerT::value_type::element_type;

    IterableSmartPtrCollection(ContainerT& container)
        : _container(container)
    {
    }

    template <typename T = ContainerT>
    typename std::enable_if<!std::is_const<T>::value, iterator>::type
        begin() noexcept
    {
        return iterator{_container.begin()};
    }

    template <typename T = ContainerT>
    typename std::enable_if<!std::is_const<T>::value, iterator>::type
        end() noexcept
    {
        return iterator{_container.end()};
    }

    auto begin() const noexcept { return const_iterator{_container.begin()}; }
    auto end() const noexcept { return const_iterator{_container.end()}; }
    auto cbegin() const noexcept { return const_iterator{_container.begin()}; }
    auto cend() const noexcept { return const_iterator{_container.end()}; }
    size_t size() const noexcept { return _container.size(); }
private:
    ContainerT& _container;
};

template <typename ContainerT>
inline auto makeIterable(ContainerT& container)
{
    return IterableSmartPtrCollection<ContainerT>(container);
}

#endif
