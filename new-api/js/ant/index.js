/* Copyright (c) 2017-2018 SKKU ESLAB, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
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

'use strict';

var ant_api_dir = process.env.ANT_BIN_DIR + '/api/';
var antNative = require(ant_api_dir + 'antnative');

var path = require('path');

exports.app = function() {
  return antNative.app();
};

exports.ml = function() {
  return antNative.ml();
};

exports.comm = function() {
  return antNative.comm();
};

exports.remoteUI = function() {
  return require(path.join(__dirname, 'remoteUI'));
}

exports.resource = function() {
  // TODO: wrapping resource API
  return antNative.resource();
}

exports.sensor = function() {
  return require(path.join(__dirname, 'sensor'));
}