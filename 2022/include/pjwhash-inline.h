/* pjwhash-inline.h - PJW hash function, inline version.
 *
 * Copyright (C) 2021-2022 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#ifndef _PJWHASH_INLINE_H
#define _PJWHASH_INLINE_H

#include "bits.h"

#define THREE_QUARTERS ((int) ((BITS_PER_INT * 3) / 4))
#define ONE_EIGHTH     ((int) (BITS_PER_INT / 8))
#define HIGH_BITS      ( ~((uint)(~0) >> ONE_EIGHTH ))

#ifndef _pjw_inline
#define _pjw_inline static inline
#endif

_pjw_inline unsigned int pjwhash(const char* str, uint length)
{
   uint hash = 0, high;

   for (uint i = 0; i < length; ++str, ++i) {
      hash = (hash << ONE_EIGHTH) + *str;
      high = hash & HIGH_BITS;
      if (high != 0) {
          hash ^= high >> THREE_QUARTERS;
          hash &= ~high;
      }
   }
   return hash;
}

#endif  /* _PJWHASH_INLINE_H */
