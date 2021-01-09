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

function CreateAppDialogView() {
  // TODO: for older browers, it may require 'dialog-polyfill'.
  // https://github.com/GoogleChrome/dialog-polyfill

  var self = this;
  this.mRootDom = document.getElementById('create-app-dialog');
  this.mConfirmButton = document.getElementById(
    'create-app-dialog-confirm-button'
  );
  this.mConfirmButton.onclick = function () {
    self.confirm();
  };
  this.mDismissButton = document.getElementById(
    'create-app-dialog-dismiss-button'
  );
  this.mDismissButton.onclick = function () {
    self.dismiss();
  };
}

CreateAppDialogView.prototype.show = function (onClickConfirm) {
  this.mOnClickConfirm = onClickConfirm;

  this.mRootDom.showModal();
};

CreateAppDialogView.prototype.confirm = function () {
  var appNameTextField = document.getElementById(
    'create-app-dialog-appname-textfield'
  );
  var appName = appNameTextField.value;
  if (
    appName === undefined ||
    typeof appName !== 'string' ||
    appName.length <= 0
  ) {
    alert('Invalid app name: ' + appName);
  }

  // Call handler
  this.mOnClickConfirm(appName);

  this.mRootDom.close();
};

CreateAppDialogView.prototype.dismiss = function () {
  this.mRootDom.close();
};
