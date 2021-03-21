/* Copyright (c) 2017-2020 SKKU ESLAB, and contributors. All rights reserved.
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

function CompanionAdapter() {
  this.mCompanionHost = undefined;
  this.mCompanionPort = undefined;
  this.mCompanionPath = undefined;
}

CompanionAdapter.prototype.setCompanionAddress = function (
  companionHost,
  companionPort,
  companionPath
) {
  this.mCompanionHost = companionHost;
  this.mCompanionPort = companionPort;
  this.mCompanionPath = companionPath;
  return true;
};

module.exports = CompanionAdapter;
