/* Copyright (c) 2017-2021 SKKU ESLAB, and contributors. All rights reserved.
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

/* AppSelectorMenuView */
function AppSelectorMenuView(onClickAppEntry, onClickCreateAppEntry) {
  View.apply(this, ['app-selector-menu']);
  this.mOnClickAppEntry = onClickAppEntry;
  this.mOnClickCreateAppEntry = onClickCreateAppEntry;

  this.mCreateAppEntry = new AppSelectorMenuEntryView(
    'add',
    'Create new app',
    onClickAppSelectorMenuCreateAppEntry
  );
  this.append(this.mCreateAppEntry);
}
AppSelectorMenuView.prototype = Object.create(View.prototype);
AppSelectorMenuView.prototype.constructor = AppSelectorMenuView;

AppSelectorMenuView.prototype.getIndex = function (appName) {
  for (var i in this.mRootDom.children) {
    var childView = this.mRootDom.children[i];
    if (childView.id == 'app-selector-menu-entry-' + appName) {
      return i;
    }
  }
  return -1;
};

AppSelectorMenuView.prototype.getAppNameAt = function (index) {
  if (index < 0) return undefined;
  var childView = this.mRootDom.children[index];
  var appName = childView.id.substring('app-selector-menu-entry-'.length);
  return appName;
};

AppSelectorMenuView.prototype.getAppsCount = function () {
  return this.mRootDom.children.length - 1;
};

AppSelectorMenuView.prototype.addApp = function (appName) {
  var self = this;
  // Check duplicated entry
  for (var i in this.mRootDom.children) {
    var childView = this.mRootDom.children[i];
    if (childView.id == 'app-selector-menu-entry-' + appName) {
      return;
    }
  }

  // Make and insert new entry
  var entry = new AppSelectorMenuEntryView(
    undefined,
    appName,
    onClickAppSelectorMenuAppEntry
  );
  this.insertBefore(entry, this.mCreateAppEntry);
};

AppSelectorMenuView.prototype.removeApp = function (appName) {
  for (var i in this.mRootDom.children) {
    var childView = this.mRootDom.children[i];
    if (childView.id == 'app-selector-menu-entry-' + appName) {
      childView.remove();
      return;
    }
  }
};

AppSelectorMenuView.prototype.show = function () {
  if (this.mRootDom.MaterialMenu !== undefined) {
    this.mRootDom.MaterialMenu.show();
  }
};

AppSelectorMenuView.prototype.hide = function () {
  if (this.mRootDom.MaterialMenu !== undefined) {
    var self = this;
    setTimeout(function () {
      self.mRootDom.MaterialMenu.hide();
    }, 500);
  }
};

AppSelectorMenuView.prototype.toggle = function () {
  if (this.mRootDom.MaterialMenu !== undefined) {
    this.mRootDom.MaterialMenu.toggle();
  }
};

function onClickAppSelectorMenuAppEntry(e) {
  var id = e.target.id;
  var appName = id.substring('app-selector-menu-entry-'.length);
  gUIController.mAppSelectorMenu.mOnClickAppEntry(appName);

  gUIController.mAppSelectorMenu.hide();
}

function onClickAppSelectorMenuCreateAppEntry(e) {
  gUIController.mAppSelectorMenu.mOnClickCreateAppEntry();

  gUIController.mAppSelectorMenu.hide();
}

/* AppSelectorMenuEntryView */
function AppSelectorMenuEntryView(iconType, title, onClickHandler) {
  View.apply(this, ['app-selector-menu-entry-' + title, 'li']);

  this.mIconType = iconType;
  this.mTitle = title;

  this.mRootDom.setAttribute('class', 'mdl-menu__item');
  this.mRootDom.onclick = onClickHandler;

  if (iconType !== undefined) {
    var icon = document.createElement('i');
    icon.setAttribute('class', 'material-icons');
    icon.setAttribute('style', 'margin-right:5px;');
    icon.innerHTML = this.mIconType;
    this.append(icon);
  }
  this.append(title);
}
AppSelectorMenuEntryView.prototype = Object.create(View.prototype);
AppSelectorMenuEntryView.prototype.constructor = AppSelectorMenuEntryView;

AppSelectorMenuEntryView.prototype.getTitle = function () {
  return this.mTitle;
};

AppSelectorMenuEntryView.prototype.getIconType = function () {
  return this.mIconType;
};
