/*
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
package org.alljoyn.bus.common;

import java.util.Arrays;

public class ECCPrivateKey {

    /**
     * The ECCPrivateKey data
     */
    private byte d[];

    public ECCPrivateKey() {}

    /**
     * ECCPrivateKey constructor
     */
    public ECCPrivateKey(byte d_key[]) {
        d = d_key;
    }

    public byte[] getD() {
        return d;
    }

    public void setD(byte[] m_d) {
        d = m_d;
    }

    @Override
    public String toString() {
        return Arrays.toString(d);
    }

    @Override
    public boolean equals(Object compObj) {
        if (this == compObj) {
            return true;
        }
        if (!(compObj instanceof ECCPrivateKey)) {
            return false;
        }
        if (compObj == null) {
            return false;
        }

        ECCPrivateKey compPublicKey = (ECCPrivateKey) compObj;

        if (d.length != compPublicKey.d.length) {
            return false;
        }

        for (int index = 0; index < d.length; index++) {
            if (d[index] != compPublicKey.d[index]) {
                return false;
            }
        }

        return true;
    }
}