
/* Copyright (c) 2012, EFPL/Blue Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *
 * This file is part of DASH <https://github.com/BlueBrain/dash>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DASH_DETAIL_SERIALIZABLE_H
#define DASH_DETAIL_SERIALIZABLE_H

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

//! Provides generic serialization implementation for classes using a pimpl.
#define SERIALIZABLEIMPL( class, oarchive, iarchive )                   \
    template<>                                                          \
    void class::save( oarchive& ar, const unsigned int version ) const  \
    {                                                                   \
        ar << *getImpl();                                               \
    }                                                                   \
                                                                        \
    template<>                                                          \
    void class::load( iarchive& ar, const unsigned int version )        \
    {                                                                   \
        ar >> *getImpl();                                               \
    }

//! Provides serialization implementation with boost.textArchive for classes
//! using a pimpl.
#define SERIALIZABLETEXTARCHIVE( class )                        \
    SERIALIZABLEIMPL( class, boost::archive::text_oarchive,     \
                             boost::archive::text_iarchive )    \


#endif // DASH_DETAIL_SERIALIZABLE_H
