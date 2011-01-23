#
# - Try to find the msgfmt executeable
#
# It will set the following variables:
#
#  MSGFMT_FOUND
#  MSGFMT_EXECUTABLE
#
###################################################################
#
#  Copyright (c) 2006, Andreas Schneider <mail@cynapses.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA  02110-1301, USA.
#
###################################################################
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
#
# * Neither the name of the cmake-modules nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

IF (MSGFMT_EXECUTABLE)
  # in cache alread?
  SET(MSGFMT_FOUND TRUE)
ELSE (MSGFMT_EXECUTABLE)
  IF (UNIX)
    FIND_PROGRAM(MSGFMT_EXECUTABLE
      NAMES
        msgfmt
      PATHS
        /usr/bin
        /usr/local/bin
    )
  ENDIF(UNIX)
  IF (WIN32)
    find_program (MSGFMT_EXECUTABLE msgfmt )
  ENDIF(WIN32)
  IF(MSGFMT_EXECUTABLE)
    SET(MSGFMT_FOUND TRUE)
  ELSE(MSGFMT_EXECUTABLE)
    MESSAGE(FATAL_ERROR "msgfmt not found - po files can't be processed")
ENDIF(MSGFMT_EXECUTABLE)
MARK_AS_ADVANCED(MSGFMT_EXECUTABLE)
ENDIF (MSGFMT_EXECUTABLE)

# vim:et ts=2 sw=2 comments=\:\#
