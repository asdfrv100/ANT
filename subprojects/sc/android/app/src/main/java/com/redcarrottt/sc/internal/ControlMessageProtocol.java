package com.redcarrottt.sc.internal;

/* Copyright (c) 2018, contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong <redcarrottt@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
public class ControlMessageProtocol {
    // Control Request Code
    class CMCode {
        public static final char kConnect = 1;
        public static final char kSleep = 2;
        public static final char kWakeup = 3;
        public static final char kDisconnect = 4;
        public static final char kPriv = 10;
        public static final char kDisconnectAck = 24;
    }

    // Priv Type Code
    public class PrivType {
        public static final char kWFDInfo = 1;
        public static final char kWFDUnknown = 999;
    }
}