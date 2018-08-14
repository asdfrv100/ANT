package com.redcarrottt.sc.wfd;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.WpsInfo;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pGroup;
import android.net.wifi.p2p.WifiP2pManager;
import android.os.Handler;
import android.os.Looper;

import com.redcarrottt.sc.OnConnectResult;
import com.redcarrottt.sc.OnDisconnectResult;
import com.redcarrottt.sc.OnDiscoverResult;
import com.redcarrottt.sc.P2PClient;
import com.redcarrottt.testapp.Logger;

class WfdP2PClient extends P2PClient {
    private static final String kTag = "WfdP2PClient";

    @Override
    protected void discoverImpl(OnDiscoverResult onDiscoverResult) {
        // Run Discover Transaction
        runDiscoverTx(onDiscoverResult);
    }

    private void runDiscoverTx(OnDiscoverResult onDiscoverResult) {
        if (sOngoingDiscover == null) {
            sOngoingDiscover = new DiscoverTransaction();
            sOngoingDiscover.run(onDiscoverResult);
        } else {
            Logger.WARN(kTag, "Already stopping core");
            onDiscoverResult.onDoneDiscover(false);
        }
    }

    private static void doneDiscoverTx(OnDiscoverResult onDiscoverResult, boolean isSuccess) {
        onDiscoverResult.onDoneDiscover(isSuccess);
        sOngoingDiscover = null;
    }

    private static DiscoverTransaction sOngoingDiscover = null;

    private class DiscoverTransaction {
        // TODO: Adding timeout on this transaction would be good.
        private OnDiscoverResult mOnDiscoverResult;

        void run(OnDiscoverResult onDiscoverResult) {
            this.mOnDiscoverResult = onDiscoverResult;
            stopP2PDiscovery();
        }

        // Step 1
        private void stopP2PDiscovery() {
            Logger.DEBUG(kTag, "Stop discovery");
            mWFDManager.stopPeerDiscovery(mWFDChannel, null);
            startP2PDiscovery();
        }

        // Step 2
        private void startP2PDiscovery() {
            Logger.DEBUG(kTag, "Start discovery");
            final Handler handler = new Handler();
            handler.postDelayed(this.mStartP2PDiscoveryRunnable, 500);
        }

        // Step 3
        private Runnable mStartP2PDiscoveryRunnable = new Runnable() {
            @Override
            public void run() {
                Logger.DEBUG(kTag, "Start discovery inner");

                // Start to listen peer change
                IntentFilter intentFilter = new IntentFilter();
                intentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
                Context context = mOwnerActivity.getApplicationContext();
                context.registerReceiver(mPeersChangedReceiver, intentFilter);

                // Discover peers
                mWFDManager.discoverPeers(mWFDChannel, null);
            }
        };

        // Step 4
        private BroadcastReceiver mPeersChangedReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                assert action != null;
                if (action.equals(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION)) {
                    Logger.DEBUG(kTag, "Peers changed event");

                    // Request peer list
                    if (!mPeerListListener.isWaiting()) {
                        mPeerListListener.setWaiting();
                        mWFDManager.requestPeers(mWFDChannel, mPeerListListener);
                    }

                    // Unregister this receiver
                    Context bContext = mOwnerActivity.getApplicationContext();
                    bContext.unregisterReceiver(this);
                }
            }
        };

        // Step 5
        private PeerListListener mPeerListListener = new PeerListListener();

        class PeerListListener implements WifiP2pManager.PeerListListener {
            private boolean mWaiting = false;

            public boolean isWaiting() {
                return this.mWaiting;
            }

            public void setWaiting() {
                this.mWaiting = true;
            }

            private void resetWaiting() {
                this.mWaiting = false;
            }

            @Override
            public void onPeersAvailable(WifiP2pDeviceList peers) {
                Logger.DEBUG(kTag, "Got peer list");
                boolean isFound = false;
                for (WifiP2pDevice p2pDevice : peers.getDeviceList()) {
                    Logger.DEBUG(kTag, "Peer: " + p2pDevice.deviceAddress + " " + p2pDevice
                            .deviceName);
                    if (p2pDevice.deviceName.equals(mTargetDevName)) {
                        mFoundWFDDevice = p2pDevice;
                        Logger.DEBUG(kTag, "Peer found");
                        isFound = true;
                        break;
                    }
                }

                this.resetWaiting();
                doneDiscoverTx(mOnDiscoverResult, isFound);
            }
        }
    }

    private WifiP2pDevice mFoundWFDDevice = null;

    // TODO: Adding timeout on this transaction would be good.
    @Override
    protected void connectImpl(OnConnectResult onConnectResult) {
        if (this.mFoundWFDDevice == null) {
            Logger.ERR(kTag, "Not found target WFD device");
            onConnectResult.onDoneConnect(false);
            return;
        }

        // Start to listen P2P connection event
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        Context context = this.mOwnerActivity.getApplicationContext();
        this.mWFDConnectionReceiver.setOnConnectResult(onConnectResult);
        context.registerReceiver(this.mWFDConnectionReceiver, intentFilter);

        // Setting P2P config
        Logger.DEBUG(kTag, "Connect WFD Peer");
        WifiP2pDevice p2pDevice = this.mFoundWFDDevice;
        WifiP2pConfig p2pConfig = new WifiP2pConfig();
        p2pConfig.deviceAddress = p2pDevice.deviceAddress;
        if (p2pDevice.wpsPbcSupported()) {
            Logger.DEBUG(kTag, "WPS Method: PBC");
            p2pConfig.wps.setup = WpsInfo.PBC;
        } else if (p2pDevice.wpsKeypadSupported()) {
            Logger.DEBUG(kTag, "WPS Method: Keypad");
            p2pConfig.wps.setup = WpsInfo.KEYPAD;
            p2pConfig.wps.pin = "12345670";
        } else if (p2pDevice.wpsDisplaySupported()) {
            Logger.DEBUG(kTag, "WPS Method: Display - Unsupported");
            onConnectResult.onDoneConnect(false);
            return;
        }
        p2pConfig.groupOwnerIntent = 0;

        // P2P connection request
        Logger.DEBUG(kTag, "Request to connect: " + p2pDevice.deviceName + " " + p2pDevice
                .deviceAddress + " (" + p2pConfig.wps.pin + ")");
        mWFDManager.connect(mWFDChannel, p2pConfig, null);
    }

    private WFDConnectionReceiver mWFDConnectionReceiver = new WFDConnectionReceiver();

    private class WFDConnectionReceiver extends BroadcastReceiver {
        private OnConnectResult mOnConnectResult = null;

        public void setOnConnectResult(OnConnectResult onConnectResult) {
            this.mOnConnectResult = onConnectResult;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            assert action != null;
            if (action.equals(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION)) {
                NetworkInfo netInfo = intent.getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);
                if (netInfo.isConnected()) {
                    WifiP2pGroup p2pGroup = intent.getParcelableExtra(WifiP2pManager
                            .EXTRA_WIFI_P2P_GROUP);
                    String goDevName = p2pGroup.getOwner().deviceName;
                    if (goDevName != null && goDevName.equals(mTargetDevName)) {
                        mOnConnectResult.onDoneConnect(true);
                    }
                }
            }
        }
    }

    @Override
    protected void disconnectImpl(OnDisconnectResult onDisconnectResult) {
        final OnDisconnectResult kOnDisconnectResult = onDisconnectResult;
        mWFDManager.removeGroup(mWFDChannel, new WifiP2pManager.ActionListener() {
            @Override
            public void onSuccess() {
                kOnDisconnectResult.onDoneDisconnect(true);
            }

            @Override
            public void onFailure(int i) {
                kOnDisconnectResult.onDoneDisconnect(false);
            }
        });
    }

    // Constructor
    WfdP2PClient(Activity ownerActivity, String targetDevName) {
        this.mOwnerActivity = ownerActivity;
        this.mTargetDevName = targetDevName;

        // Initialize Android WFD Manager Channel
        this.mWFDManager = (WifiP2pManager) this.mOwnerActivity.getApplicationContext()
                .getSystemService(Context.WIFI_P2P_SERVICE);
        if (this.mWFDManager == null) {
            Logger.ERR(kTag, "Failed to initialize Wi-fi Direct Manager");
            return;
        }

        this.mWFDChannel = this.mWFDManager.initialize(this.mOwnerActivity, Looper.getMainLooper
                (), null);
        if (this.mWFDChannel == null) {
            Logger.ERR(kTag, "Failed to initialize Wi-fi Direct Manager");
        }
    }

    // Attributes
    private Activity mOwnerActivity;
    private String mTargetDevName;

    // Components
    private WifiP2pManager mWFDManager;
    private WifiP2pManager.Channel mWFDChannel;
}
