'use strict';

var _ = require('lodash');
var $ = require('preconditions').singleton();
var log = require('npmlog');
log.debug = log.verbose;

var Insight = require('./blockchainexplorers/insight');
var Common = require('./common');
var Constants = Common.Constants,
  Defaults = Common.Defaults,
  Utils = Common.Utils;

var PROVIDERS = {
  'insight': {
    'livenet': 'https://digiexplorer.info:443',
    'testnet': 'https://test-insight.bitpay.com:443',
  },
};

function BlockChainExplorer(opts) {
  $.checkArgument(opts);

  var provider = opts.provider || 'insight';
  var network = opts.network || 'livenet';

  $.checkState(PROVIDERS[provider], 'Provider ' + provider + ' not supported');
  $.checkState(_.contains(_.keys(PROVIDERS[provider]), network), 'Network ' + network + ' not supported by this provider');

  var url = opts.url || PROVIDERS[provider][network];

  

  switch (provider) {
    case 'insight':
      return new Insight({
        network: network,
        url: url,
        apiPrefix: opts.apiPrefix,
        userAgent: opts.userAgent,
        addressFormat: opts.addressFormat,
      });
    default:
      throw new Error('Provider ' + provider + ' not supported.');
  };
};

module.exports = BlockChainExplorer;