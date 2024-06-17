/**
 * Wrappers for the public implementations retrieved from github, to present the same
 * interface (PMA_Interface)
 *
 * Copyright (C) 2018 Dean De Leo, email: dleo[at]cwi.nl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef PMA_EXTERNAL_HPP_
#define PMA_EXTERNAL_HPP_

#include "interface.hpp"
#include "iterator.hpp"

namespace pma {


/*****************************************************************************
 *                                                                           *
 *   rfdavid (drui) pma.hpp                                                  *
 *                                                                           *
 *****************************************************************************/

class PMA_Drui : public Interface {
private:
    void* impl_r;

public:
    PMA_Drui(std::uint64_t capacity = 64);

    virtual ~PMA_Drui();

    virtual void insert(int64_t key, int64_t value);

    virtual int64_t find(int64_t key) const;

    virtual std::size_t size() const;

    // not implemented
    virtual pma::Interface::SumResult sum(int64_t min, int64_t max) const;

    std::unique_ptr<Iterator> iterator() const;

    virtual void dump() const;
};

/*****************************************************************************
 *                                                                           *
 *   Gaurav Menghani / impl1.hpp                                             *
 *   This implementation is broken, it hits a seg~ fault somewhere in the    *
 *   unit tests. Ignored in the current experiments                          *
 *                                                                           *
 *****************************************************************************/
class PMA_Menghani_1 : public Interface {
private:
    void* impl;

public:
    PMA_Menghani_1();

    virtual ~PMA_Menghani_1();

    virtual void insert(int64_t key, int64_t value);

    virtual int64_t find(int64_t key) const;

    virtual std::size_t size() const;

    // not implemented
    virtual pma::Interface::SumResult sum(int64_t min, int64_t max) const;

    std::unique_ptr<Iterator> iterator() const;

    virtual void dump() const;
};

/*****************************************************************************
 *                                                                           *
 *   Gaurav Menghani / impl2.hpp                                             *
 *                                                                           *
 *****************************************************************************/

class PMA_Menghani_2 : public Interface {
private:
    void* implementation;

public:
    PMA_Menghani_2(std::size_t capacity = 64);

    virtual ~PMA_Menghani_2();

    virtual void insert(int64_t key, int64_t value);

    virtual int64_t find(int64_t key) const;

    virtual std::size_t size() const;

    // not implemented
    virtual pma::Interface::SumResult sum(int64_t min, int64_t max) const;

    std::unique_ptr<Iterator> iterator() const;

    virtual void dump() const;
};


/*****************************************************************************
 *                                                                           *
 *   Pablo Montes                                                            *
 *                                                                           *
 *****************************************************************************/

class PMA_Montes : public InterfaceRQ {
private:
    void* m_implementation;

    // get the begin (inclusive) & end (exclusive) index for the interval [min, max]
    std::pair<int64_t, int64_t> find_interval(int64_t min, int64_t max) const;

public:
    PMA_Montes();

    ~PMA_Montes();

    void insert(int64_t key, int64_t value);

    int64_t find(int64_t key) const;

    std::size_t size() const;

    virtual std::size_t capacity() const;

    std::unique_ptr<pma::Iterator> find(int64_t min, int64_t max) const;

    std::unique_ptr<pma::Iterator> iterator() const;

    pma::Interface::SumResult sum(int64_t min, int64_t max) const;

    // For debugging purposes only
    void dump() const;

    virtual void fulldump() const;
};

/*****************************************************************************
 *                                                                           *
 *   Justin Raizes                                                           *
 *                                                                           *
 *****************************************************************************/

class PMA_Raizes : public Interface {
private:
    void* implementation;

public:
    PMA_Raizes();

    ~PMA_Raizes();

    void insert(int64_t key, int64_t value);

    int64_t find(int64_t key) const;

    std::size_t size() const;

    // not implemented
    pma::Interface::SumResult sum(int64_t min, int64_t max) const;

    std::unique_ptr<Iterator> iterator() const;

    // For debugging purposes only
    void dump() const;
};

}

#endif /* PMA_EXTERNAL_HPP_ */
