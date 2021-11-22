## `tailer` - tail entire directories at once using inotify

This utility is based on Erik Zenker's inotify-cpp library for monitoring file
create/modify/remove events. That library does all the heavy-lifting.  See:
https://github.com/erikzenker/inotify-cpp

Conceptually `tailer` is equivalent to `tail -F -n 0 *`, which also tails
multiple files at once, but with one limitation, it won't pick up newly created
files. That limitation was the main reason for making this utility.

This program does not have any CPU usage, only when it reacts to inotify events.


## Implementation

The implementation is very simple, basically the inotify-cpp `example.cpp` with
some small changes.


## Binary distribution

I've tagged version v1.0.1. and uploaded a release on GitHub:

https://github.com/rayburgemeestre/tailer/releases/tag/v1.0.1

If you trust me you can use the binary, otherwise you have to compile it
yourself, with instructions below. :)


## Compile instructions for Ubuntu 20.04

Sorry, currently only tested on Ubuntu 20.04. If you get this thing working on
another distro, please let me know how, so I can extend the instructions.

    git clone --recursive https://github.com/rayburgemeestre/tailer
    cd tailer
    make prepare
    make build
    make install


## Usage example

For this example `tailer` reads 226 different files present in `$PWD/dist/logs`.

    trigen@ideapad:~/owt-server> tailer ./dist/logs
    Initialized 226 files.
    Listening with inotify.. Press Control+C to stop the process.
    ---

Nothing is printed, only when those files are appended to, or when they are truncated.
After waitig for a while, some activity happens, and the output continues.

    trigen@ideapad:~/owt-server> tailer ./dist/logs
    Initialized 226 files.
    Listening with inotify.. Press Control+C to stop the process.
    ---
    management-api.pid: *** truncated ***
    management-api.pid: 287
    management-api.stdout: *** truncated ***
    cluster-manager.stdout: *** truncated ***
    cluster-manager.stdout: 2021-11-09 16:25:17.830  - INFO: AmqpClient - Connection to rabbitMQ server error { Error: connect ECONNREFUSED 127.0.0.1:5672
    cluster-manager.stdout:     at TCPConnectWrap.afterConnect [as oncomplete] (net.js:1191:14)
    cluster-manager.stdout:   errno: 'ECONNREFUSED',
    cluster-manager.stdout:   code: 'ECONNREFUSED',
    cluster-manager.stdout:   syscall: 'connect',
    cluster-manager.stdout:   address: '127.0.0.1',
    cluster-manager.stdout:   port: 5672 }
    management-api.stdout: 2021-11-09 16:25:18.143  - INFO: ManagementServer - Worker 298 started
    management-api.stdout: 2021-11-09 16:25:18.150  - INFO: ManagementServer - Worker 305 started
    management-api.stdout: 2021-11-09 16:25:18.156  - INFO: ManagementServer - Worker 303 started
    management-api.stdout: 2021-11-09 16:25:18.177  - INFO: ManagementServer - Worker 311 started
    management-api.stdout: 2021-11-09 16:25:18.179  - INFO: RPC - Connected to rabbitMQ server
    management-api.stdout: 2021-11-09 16:25:18.183  - INFO: RPC - Exchange owtRpc is open
    management-api.stdout: 2021-11-09 16:25:18.186  - INFO: RPC - ClientQueue amq.gen-MFp3I6G4zZhuGUv-3eaUqA is open
    management-api.stdout: 2021-11-09 16:25:18.190  - INFO: RPC - Connected to rabbitMQ server
    management-api.stdout: 2021-11-09 16:25:18.192  - INFO: RPC - Exchange owtRpc is open
    management-api.stdout: 2021-11-09 16:25:18.194  - INFO: RPC - Connected to rabbitMQ server
    management-api.stdout: 2021-11-09 16:25:18.195  - INFO: RPC - ClientQueue amq.gen-s_oW6nT0D_VkxqbyUolsbw is open
    management-api.stdout: 2021-11-09 16:25:18.197  - INFO: RPC - Exchange owtRpc is open
    management-api.stdout: 2021-11-09 16:25:18.199  - INFO: RPC - ClientQueue amq.gen-bHCqxpzoN5gD7GzxM2yisg is open
    management-api.stdout: 2021-11-09 16:25:18.212  - INFO: RPC - Connected to rabbitMQ server
    management-api.stdout: 2021-11-09 16:25:18.214  - INFO: RPC - Exchange owtRpc is open
    management-api.stdout: 2021-11-09 16:25:18.217  - INFO: RPC - ClientQueue amq.gen-PLJzbx8yRJSb3YPv_buaxg is open
    portal.stdout: *** truncated ***
    cluster-manager.stdout: 2021-11-09 16:25:18.837  - INFO: AmqpClient - Connecting to rabbitMQ server OK, options: { host: 'localhost', port: 5672 }
    cluster-manager.stdout: 2021-11-09 16:25:18.844  - INFO: Main - Cluster manager up! id: 992150773
    cluster-manager.stdout: 2021-11-09 16:25:18.845  - INFO: ClusterManager - Run as candidate.
    cluster-manager.stdout: 2021-11-09 16:25:19.040  - INFO: ClusterManager - Run as master.
    cluster-manager.stdout: 2021-11-09 16:25:19.048  - INFO: ClusterManager - Cluster manager is in service as master!
    portal.stdout: 2021-11-09 16:25:19.082  - INFO: AmqpClient - Connecting to rabbitMQ server OK, options: { host: 'localhost', port: 5672 }
    portal.stdout: 2021-11-09 16:25:19.093  - INFO: Main - portal initializing as rpc client ok
    portal.stdout: 2021-11-09 16:25:19.128  - INFO: Main - portal join cluster ok, with rpcID: portal-47b414af4b63fbb0f203@172.18.0.2
    portal.stdout: 2021-11-09 16:25:19.128  - INFO: ClusterWorker - Join cluster owt-cluster OK.
    portal.stdout: 2021-11-09 16:25:19.132  - INFO: Main - portal initializing as rpc server ok
    portal.stdout: 2021-11-09 16:25:19.136  - INFO: Main - portal-47b414af4b63fbb0f203@172.18.0.2 as monitor ready
    portal.stdout: 2021-11-09 16:25:19.412  - INFO: Main - start socket.io server ok.
    ^C

The program is stopped with CONTROL+C.
