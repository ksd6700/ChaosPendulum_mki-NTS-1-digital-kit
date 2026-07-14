# Convenience Makefile for a KORG logue-sdk checkout.
#
# Recommended:
#   1. Copy this directory to:
#        logue-sdk/platform/nutekt-digital/pendy
#   2. Build from that directory:
#        make install
#
# If this include does not match your SDK checkout, copy the Makefile from
# platform/nutekt-digital/dummy-osc into this directory and keep project.mk,
# manifest.json, and pendy_osc.cpp.

PLATFORMDIR ?= ../..

include $(PLATFORMDIR)/nutekt-digital/dummy-osc/Makefile
