/**
 * @file index.js
 *
 * Created by mmhunter on 16/10/2017.
 */
module.exports = {
    SyncIPCServer: require('./lib/server').SyncIPCServer,
    SyncIPCClient: require('./lib/client').SyncIPCClient,
};