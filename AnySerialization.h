
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

#ifndef DASH_DETAIL_ANYSERIALIZATION_H
#define DASH_DETAIL_ANYSERIALIZATION_H

#include "Any.h"

#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT_KEY2(dash::detail::Any::holder<short>,
                        "a5cf8ba4-aacc-439a-8f3e-726813a5c3a9");
BOOST_CLASS_EXPORT_KEY2(dash::detail::Any::holder<unsigned short>,
                        "90a49245-c935-4eef-959c-f30d77849231");
BOOST_CLASS_EXPORT_KEY2(dash::detail::Any::holder<int>,
                        "0019edb4-4d23-43ea-97d1-5452d5dd0381");
BOOST_CLASS_EXPORT_KEY2(dash::detail::Any::holder<unsigned int>,
                        "7e458b15-e2ee-48ac-a6e4-5db7344bc0d0");
BOOST_CLASS_EXPORT_KEY2(dash::detail::Any::holder<long>,
                        "1b1dce63-523e-4232-8bf0-2829e161c57c");
BOOST_CLASS_EXPORT_KEY2(dash::detail::Any::holder<unsigned long>,
                        "3e92decc-b8c0-4574-ae18-087b28984067");
BOOST_CLASS_EXPORT_KEY2(dash::detail::Any::holder<bool>,
                        "166234f5-0819-4822-866e-0812f2e98002");
BOOST_CLASS_EXPORT_KEY2(dash::detail::Any::holder<float>,
                        "7ce8eb79-3a0e-47b4-a793-06d16db8218b");
BOOST_CLASS_EXPORT_KEY2(dash::detail::Any::holder<double>,
                        "2ec87dca-935f-459b-aeac-1420f2d5fd89");
BOOST_CLASS_EXPORT_KEY2(dash::detail::Any::holder<std::string>,
                        "01ba025d-656d-474f-8448-111b6baf4291");

#endif
