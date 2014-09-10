
# Copyright (c) 2014 Stefan.Eilemann@epfl.ch

# Prefer dnssd (Bonjour) on Apple, avahi on all other platforms.
if(DNSSD_FOUND AND avahi-client_FOUND)
  if(APPLE)
    message(STATUS "Disabling avahi over preferred dnssd implementation")
    set(avahi-client_FOUND)
  else()
    message(STATUS "Disabling dnssd over preferred avahi implementation")
    set(DNSSD_FOUND)
  endif()
endif()
