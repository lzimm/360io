# -*- test-case-name: hack.web2.test.test_cgi,hack.web2.test.test_http -*-
# See LICENSE for details.

"""
Various backend channel implementations for web2.
"""
from hack.web2.channel.cgi import startCGI
from hack.web2.channel.scgi import SCGIFactory
from hack.web2.channel.http import HTTPFactory
from hack.web2.channel.fastcgi import FastCGIFactory

__all__ = ['startCGI', 'SCGIFactory', 'HTTPFactory', 'FastCGIFactory']
