<?php

namespace ccxt;

// PLEASE DO NOT EDIT THIS FILE, IT IS GENERATED AND WILL BE OVERWRITTEN:
// https://github.com/ccxt/ccxt/blob/master/CONTRIBUTING.md#how-to-contribute-code

use Exception as Exception; // a common import

class hadax extends huobipro {

    public function describe () {
        return array_replace_recursive (parent::describe (), array (
            'id' => 'hadax',
            'name' => 'HADAX',
            'countries' => array ( 'CN' ),
            'hostname' => 'api.hadax.com',
            'urls' => array (
                'logo' => 'https://user-images.githubusercontent.com/1294454/38059952-4756c49e-32f1-11e8-90b9-45c1eccba9cd.jpg',
                'api' => 'https://api.hadax.com',
                'www' => 'https://www.hadax.com',
                'doc' => 'https://github.com/huobiapi/API_Docs/wiki',
            ),
            'has' => array (
                'fetchCurrencies' => false,
            ),
            'api' => array (
                'public' => array (
                    'get' => array (
                        'hadax/common/symbols', // 查询系统支持的所有交易对
                        'hadax/common/currencys', // 查询系统支持的所有币种
                        'common/timestamp', // 查询系统当前时间
                        'hadax/settings/currencys', // ?language=en-US
                    ),
                ),
                'private' => array (
                    'get' => array (
                        'account/accounts', // 查询当前用户的所有账户(即account-id)
                        'hadax/account/accounts/{id}/balance', // 查询指定账户的余额
                        'order/orders/{id}', // 查询某个订单详情
                        'order/orders/{id}/matchresults', // 查询某个订单的成交明细
                        'order/orders', // 查询当前委托、历史委托
                        'order/matchresults', // 查询当前成交、历史成交
                        'dw/withdraw-virtual/addresses', // 查询虚拟币提现地址
                        'dw/deposit-virtual/addresses',
                        'query/deposit-withdraw',
                        'margin/loan-orders', // 借贷订单
                        'margin/accounts/balance', // 借贷账户详情
                    ),
                    'post' => array (
                        'hadax/order/orders/place', // 创建并执行一个新订单 (一步下单， 推荐使用)
                        'order/orders', // 创建一个新的订单请求 （仅创建订单，不执行下单）
                        'order/orders/{id}/place', // 执行一个订单 （仅执行已创建的订单）
                        'order/orders/{id}/submitcancel', // 申请撤销一个订单请求
                        'order/orders/batchcancel', // 批量撤销订单
                        'dw/balance/transfer', // 资产划转
                        'dw/withdraw/api/create', // 申请提现虚拟币
                        'dw/withdraw-virtual/create', // 申请提现虚拟币
                        'dw/withdraw-virtual/{id}/place', // 确认申请虚拟币提现
                        'dw/withdraw-virtual/{id}/cancel', // 申请取消提现虚拟币
                        'dw/transfer-in/margin', // 现货账户划入至借贷账户
                        'dw/transfer-out/margin', // 借贷账户划出至现货账户
                        'margin/orders', // 申请借贷
                        'margin/orders/{id}/repay', // 归还借贷
                    ),
                ),
            ),
            'exceptions' => array (
                'not-allow-entry-hadax' => '\\ccxt\\PermissionDenied',
            ),
            'options' => array (
                'fetchMarketsMethod' => 'publicGetHadaxCommonSymbols',
                'fetchBalanceMethod' => 'privateGetHadaxAccountAccountsIdBalance',
                'createOrderMethod' => 'privatePostHadaxOrderOrdersPlace',
            ),
            'commonCurrencies' => array (
                'FAIR' => 'FairGame',
                'GET' => 'Themis',
                'HOT' => 'Hydro Protocol',
            ),
        ));
    }
}
