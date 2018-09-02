package com.redcarrottt.testapp;

/* Copyright (c) 2017-2018. All rights reserved.
 *  Gyeonghwan Hong (redcarrottt@gmail.com)
 *  Eunsoo Park (esevan.park@gmail.com)
 *  Injung Hwang (sinban04@gmail.com)
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

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ListView;
import android.widget.TextView;

import com.redcarrottt.sc.api.API;
import com.redcarrottt.sc.api.OnStartSCResult;
import com.redcarrottt.sc.api.OnStopSCResult;
import com.redcarrottt.sc.internal.bt.BtClientAdapter;
import com.redcarrottt.sc.internal.wfd.WfdClientAdapter;

import java.util.ArrayList;

public class MainActivity extends AppCompatActivity implements LogReceiver.Callback {
    private static final String kTag = "MainActivity";

    // Components
    private ArrayList<LogListViewItem> mLogListViewData = new ArrayList<>();
    private LogListViewAdapter mLogListViewAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // Initialize SwitchProfileButton
        Button switchProfileButton = (Button) findViewById(R.id.switchProfileButton);
        switchProfileButton.setOnClickListener(onClickSwitchProfileButton);
        this.updateSwitchProfileButton();

        // Initialize StartButton
        Button startButton = (Button) findViewById(R.id.startButton);
        startButton.setOnClickListener(onClickStartButton);

        // Initialize BtDataCheckBox
        CheckBox btDataCheckBox = (CheckBox) findViewById(R.id.btDataCheckbox);
        btDataCheckBox.setOnCheckedChangeListener(onChangeBtDataCheckBox);

        // Initialize WfdDataCheckBox
        CheckBox wfdDataCheckBox = (CheckBox) findViewById(R.id.wfdDataCheckbox);
        wfdDataCheckBox.setOnCheckedChangeListener(onChangeWfdDataCheckBox);

        // Initialize LogListView
        this.mLogListViewAdapter = new LogListViewAdapter(this, this.mLogListViewData);
        ListView logListView = (ListView) findViewById(R.id.logListView);
        logListView.setAdapter(this.mLogListViewAdapter);

        // Initialize Log Receiver
        Logger.setDefaultContext(this);
        LogReceiver logReceiver = new LogReceiver(this);
        IntentFilter broadcastIntentFilter = new IntentFilter();
        broadcastIntentFilter.addAction(LogReceiver.kAction);
        this.registerReceiver(logReceiver, broadcastIntentFilter);

        // Require permissions for SC
        this.requestPermissions();
    }

    private View.OnClickListener onClickStartButton = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            // Initialize SC
            initializeCommunication();
        }
    };

    private String[] mProfiles = {"Lab", "Home"};
    private int mPresentProfile = 0;
    private View.OnClickListener onClickSwitchProfileButton = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            final int kMaxProfiles = mProfiles.length;
            mPresentProfile = (mPresentProfile + 1) % kMaxProfiles;
            updateSwitchProfileButton();
        }
    };

    private void updateSwitchProfileButton() {
        Button switchProfileButton = (Button) findViewById(R.id.switchProfileButton);
        switchProfileButton.setText(this.mProfiles[this.mPresentProfile] + " Profile");
    }

    private boolean mIsBtDataChecked = true;
    private CheckBox.OnCheckedChangeListener onChangeBtDataCheckBox = new CheckBox
            .OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton self, boolean isChecked) {
            mIsBtDataChecked = isChecked;
            if(!mIsBtDataChecked && !mIsWfdDataChecked) {
                mIsBtDataChecked = true;
                self.setChecked(true);
            }
        }
    };

    private boolean mIsWfdDataChecked = true;
    private CheckBox.OnCheckedChangeListener onChangeWfdDataCheckBox = new CheckBox
            .OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton self, boolean isChecked) {
            mIsWfdDataChecked = isChecked;
            if(!mIsBtDataChecked && !mIsWfdDataChecked) {
                mIsWfdDataChecked = true;
                self.setChecked(true);
            }
        }
    };

    private void requestPermissions() {
        if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.INTERNET) !=
                PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission
                    .INTERNET, Manifest.permission.ACCESS_NETWORK_STATE, Manifest.permission
                    .ACCESS_WIFI_STATE, Manifest.permission.CHANGE_WIFI_STATE, Manifest
                    .permission.CHANGE_NETWORK_STATE, Manifest.permission
                    .ACCESS_COARSE_LOCATION}, 0);
        }
    }

    private void initializeCommunication() {
        String btAddress = "";
        switch (this.mPresentProfile) {
            case 0:
                btAddress = "B8:27:EB:D9:FA:85";
                break;
            case 1:
                btAddress = "B8:27:EB:77:C3:4A";
                break;
        }

        if(!this.mIsBtDataChecked && !this.mIsWfdDataChecked) {
            Logger.ERR(kTag, "No data adapter is selected!");
            return;
        } else if(btAddress == "") {
            Logger.ERR(kTag, "No bluetooth address is defined!");
            return;
        }

        // Setting adapters
        BtClientAdapter btControl = new BtClientAdapter(2345, "Control", btAddress,
                "150e8400-1234-41d4-a716-446655440000", this);
        API.registerControlAdapter(btControl);

        if(this.mIsBtDataChecked) {
            BtClientAdapter btData = new BtClientAdapter(3333, "Data/BT", btAddress,
                    "150e8400-1234-41d4-a716-446655440001", this);
            API.registerDataAdapter(btData);
        }
        if(this.mIsWfdDataChecked) {
            WfdClientAdapter wfdData = new WfdClientAdapter(3456, "Data/WFD", 3456, this);
            API.registerDataAdapter(wfdData);
        }

        // Start the selective connection
        API.startSC(onStartSCResult);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        API.stopSC(onStopSCResult);
    }

    private OnStartSCResult onStartSCResult = new OnStartSCResult() {
        private int mRetries = 0;
        private static final int kMaxRetries = 5;

        @Override
        public void onDoneStartSC(boolean isSuccess) {
            if (isSuccess) {
                // Starting Selective Connection is done
                ReceivingThread receivingThread = new ReceivingThread();
                receivingThread.start();
            } else {
                // Retry to start SC
                if (mRetries++ < kMaxRetries) {
                    Logger.VERB(kTag, "Failed to start SC... Retry to start.");
                    try {
                        Thread.sleep(5000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    API.startSC(onStartSCResult);
                }
            }
        }
    };

    private OnStopSCResult onStopSCResult = new OnStopSCResult() {
        @Override
        public void onDoneStopSC(boolean isSuccess) {
            if (!isSuccess) {
                Log.e(kTag, "Stopping Selective Connection failed");
            }
        }
    };

    private class ReceivingThread extends Thread {
        private static final String kTag = "Recv";
        private boolean mIsAlive;

        ReceivingThread() {
            this.mIsAlive = false;
        }

        public void run() {
            this.mIsAlive = true;
            byte[] buf = new byte[100 * 1024 * 1024];
            String sending_buf = "ACK"; /* Ack Message */

            while (this.mIsAlive) {
                int receivedLength = API.receive(buf);
                Logger.VERB(kTag, "Received: Size=" + receivedLength);
                int sentLength = API.send(sending_buf.getBytes(), sending_buf.length());
                Logger.VERB(kTag, "Sent: Size=" + sentLength);
            }
        }

        public void terminate() {
            this.mIsAlive = false;
        }
    }

    @Override
    public void onLogMessage(final int logLevel, final String logMessage) {
        final int kPrintThreshold = LogLevel.VERB;
        final String kTag = "LOG";
        this.runOnUiThread(new Runnable() {
            @SuppressLint("SetTextI18n")
            @Override
            public void run() {
                String printMessage = logMessage;
                int color = Color.BLACK;
                switch (logLevel) {
                    case LogLevel.ERR:
                        Log.e(kTag, logMessage);
                        printMessage = "[E]" + printMessage;
                        color = Color.RED;
                        break;
                    case LogLevel.WARN:
                        Log.w(kTag, logMessage);
                        printMessage = "[W]" + printMessage;
                        color = Color.parseColor("#FF8000");
                        break;
                    case LogLevel.VERB:
                        Log.i(kTag, logMessage);
                        printMessage = "[V]" + printMessage;
                        color = Color.parseColor("#0072C1");
                        break;
                    case LogLevel.DEBUG:
                        Log.d(kTag, logMessage);
                        printMessage = "[D]" + printMessage;
                        color = Color.parseColor("#5C676F");
                        break;
                }

                if (logLevel <= kPrintThreshold) {
                    mLogListViewData.add(0, new LogListViewItem(printMessage, color));
                    mLogListViewAdapter.notifyDataSetChanged();
                }
            }
        });
    }
}

class LogListViewItem {
    private String mText;
    private int mColor;

    public String getText() {
        return this.mText;
    }

    public int getColor() {
        return this.mColor;
    }

    LogListViewItem(String text, int color) {
        this.mText = text;
        this.mColor = color;
    }
}

class LogListViewAdapter extends BaseAdapter {
    private LayoutInflater mInflater;
    private int mLayout;
    private ArrayList<LogListViewItem> mData;

    LogListViewAdapter(Context context, ArrayList<LogListViewItem> data) {
        this.mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        this.mLayout = R.layout.item;
        this.mData = data;
    }

    @Override
    public int getCount() {
        return this.mData.size();
    }

    @Override
    public String getItem(int position) {
        return this.mData.get(position).getText();
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
            convertView = mInflater.inflate(this.mLayout, parent, false);
        }

        LogListViewItem item = this.mData.get(position);
        TextView contentsTextView = (TextView) convertView.findViewById(R.id.contentsTextView);
        contentsTextView.setText(item.getText());
        contentsTextView.setTextColor(item.getColor());
        return contentsTextView;
    }
}