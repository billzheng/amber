#pragma once
#include "fix_common/field_ptr.hpp"
#include "fix_common/fix_istream.hpp"
#include <stdint.h>

namespace dinobot
{
  namespace fix
  {
    enum class side_type_e : char
    {
      buy  = '1',
      sell = '2'
    };
    enum class msg_type_e : char
    {
      heartbeat                      = '0',
      test_request                   = '1',
      resend_request                 = '2',
      reject                         = '3',
      sequence_reset                 = '4',
      logout                         = '5',
      execution_report               = '8',
      order_cancel_reject            = '9',
      logon                          = 'A',
      order_single                   = 'D',
      order_cancel_request           = 'F',
      order_cancel_replace_request   = 'G',
      order_status_request           = 'H',
      dont_know_trade                = 'Q',
      quote_request                  = 'R',
      quote                          = 'S',
      quote_cancel                   = 'Z',
      security_definition            = 'd',
      trading_session_status_request = 'g',
      trading_session_status         = 'h',
      mass_quote                     = 'i'
    };

    enum class encrypt_method_e : char
    {
      none = '0'
    };

    enum class cancel_orders_on_disconnect_e : char
    {
      cancel_all_open_orders                   = 'Y',
      cancel_open_orders_placed_during_session = 'S'
    };

    enum class self_trade_prevent_prevention_e : char
    {
      decrement             = 'D',
      cance_reseting_order  = 'O',
      cancel_incoming_order = 'N',
      cancel_both_orders    = 'B'
    };

    enum class time_inforce_e : char
    {
      good_till_cancel    = '1',
      immediate_or_cancel = '3',
      fill_or_kill        = '4',
      /* The post-only flag (P) indicates that the order should only make
         liquidity.
         If any part of the order results in taking liquidity, the order will be
         rejected and no part of it will execute.
         Open Post-Only orders will be treated as Good Till Cancel.
      */
      post_only = 'P'
    };

    enum class order_type_e : char
    {
      market      = '1',
      limit       = '2',
      stop_market = '3',
      stop_limit  = '4'
    };

    enum class exec_type_e : char
    {
      new_order     = '0',
      fill          = '1',
      done          = '3',
      canceled      = '4',
      stopped       = '7',
      rejected      = '8',
      order_changed = 'D',
      order_status  = 'I'
    };

    struct header_reader_t
    {
      using begin_string_t   = field_ptr_t<8>;
      using sender_comp_id_t = field_ptr_t<49>;
      using target_comp_id_t = field_ptr_t<56>;

      header_reader_t() = default;
      fix_istream_t &read(fix_istream_t &fs)
      {
        while (fs.has_next())
        {
          auto const field = fs.get_current();
          switch (std::get<0>(field))
          {
          case begin_string_t::get_id():
            begin_string.set_pointers(field);
            break;
          case sender_comp_id_t::get_id():
            sender_comp_id.set_pointers(field);
            break;
          case target_comp_id_t::get_id():
            target_comp_id.set_pointers(field);
            break;
          default:
            return fs;
          }
        }

        return fs;
      }

      void reset()
      {
        begin_string.reset();
        sender_comp_id.reset();
        target_comp_id.reset();
      }
      begin_string_t   begin_string{};
      sender_comp_id_t sender_comp_id{};
      target_comp_id_t target_comp_id{};
    };
    /*
        struct logon_t
        {
          using encrypt_t = field_ptr_t<98>;
          using heartbeat_t = field_ptr_t<108>;
          using password_t = field_ptr_t<554>;
          using raw_data_t = field_ptr_t<96>;
          using cancel_order_on_disconnect_t = field_ptr_t<8013>;
          using dropcopy_flag_t = field_ptr_t<9406>;

          logon_t() = default;
        };
    */
    struct reject_reader_t
    {
      using ref_seq_num_t              = field_ptr_t<45>;
      using ref_tag_id_t               = field_ptr_t<371>;
      using ref_msg_type_t             = field_ptr_t<372>;
      using text_t                     = field_ptr_t<58>;
      using session_rejection_reason_t = field_ptr_t<373>;

      fix_istream_t &read(fix_istream_t &fs)
      {
        while (fs.has_next())
        {
          auto const field = fs.get_current();
          switch (std::get<0>(field))
          {
          case ref_seq_num_t::get_id():
            ref_seq_num.set_pointers(field);
            break;
          case ref_tag_id_t::get_id():
            ref_tag_id.set_pointers(field);
            break;
          case ref_msg_type_t::get_id():
            ref_msg_type.set_pointers(field);
            break;
          case text_t::get_id():
            text.set_pointers(field);
            break;
          case session_rejection_reason_t::get_id():
            reject_reason.set_pointers(field);
            break;
          default:
            return fs;
          }
        }
        return fs;
      }

      void reset()
      {
        ref_seq_num.reset();
        ref_tag_id.reset();
        ref_msg_type.reset();
        text.reset();
        reject_reason.reset();
      }

      ref_seq_num_t              ref_seq_num{};
      ref_tag_id_t               ref_tag_id{};
      ref_msg_type_t             ref_msg_type{};
      text_t                     text{};
      session_rejection_reason_t reject_reason{};
    };

    struct order_cancel_reject_reader_t
    {
      using clord_id_t            = field_ptr_t<11>;
      using order_id_t            = field_ptr_t<37>;
      using orig_clord_id_t       = field_ptr_t<41>;
      using ord_status_t          = field_ptr_t<39>;
      using cx_rej_reason_t       = field_ptr_t<102>;
      using cxl_rej_response_to_t = field_ptr_t<434>;

      fix_istream_t &read(fix_istream_t &fs)
      {
        while (fs.has_next())
        {
          auto const field = fs.get_current();
          switch (std::get<0>(field))
          {
          case clord_id_t::get_id():
            clord_id.set_pointers(field);
            break;
          case order_id_t::get_id():
            order_id.set_pointers(field);
            break;
          case orig_clord_id_t::get_id():
            orig_clord_id.set_pointers(field);
            break;
          case ord_status_t::get_id():
            ord_status.set_pointers(field);
            break;
          case cx_rej_reason_t::get_id():
            cx_rej_reason.set_pointers(field);
            break;
          case cxl_rej_response_to_t::get_id():
            cx_rej_response_to.set_pointers(field);
            break;
          default:
            return fs;
          }
        }
        return fs;
      }

      void reset()
      {
        clord_id.reset();
        order_id.reset();
        orig_clord_id.reset();
        ord_status.reset();
        cx_rej_reason.reset();
        cx_rej_response_to.reset();
      }

      clord_id_t            clord_id{};
      order_id_t            order_id{};
      orig_clord_id_t       orig_clord_id{};
      ord_status_t          ord_status{};
      cx_rej_reason_t       cx_rej_reason{};
      cxl_rej_response_to_t cx_rej_response_to{};
    };

    struct test_request_reader_t
    {
      using test_req_id_t = field_ptr_t<112>;

      test_request_reader_t() = default;
      fix_istream_t &read(fix_istream_t &fs)
      {
        while (fs.has_next())
        {
          auto const field = fs.get_current();
          switch (std::get<0>(field))
          {
          case test_req_id_t::get_id():
            test_req_id.set_pointers(field);
            break;
          default:
            return fs;
          }
        }

        return fs;
      }

      void reset() { test_req_id.reset(); }

      test_req_id_t test_req_id{};
    };

    struct execution_report_reader_t
    {
      using clord_id_t       = field_ptr_t<11>;
      using order_id_t       = field_ptr_t<37>;
      using symbol_t         = field_ptr_t<55>;
      using side_t           = field_ptr_t<54>;
      using last_shares_t    = field_ptr_t<32>;
      using price_t          = field_ptr_t<44>;
      using order_qty_t      = field_ptr_t<38>;
      using cash_order_qty_t = field_ptr_t<152>;
      using transact_time_t  = field_ptr_t<60>;
      /// 1: partial filled
      /// D: self-trade prevention
      using exec_type_t           = field_ptr_t<150>;
      using ord_status_t          = field_ptr_t<39>;
      using ord_rej_reason_t      = field_ptr_t<103>;
      using no_misc_fees_t        = field_ptr_t<136>;
      using misc_fee_amt_t        = field_ptr_t<137>;
      using misc_fee_type_t       = field_ptr_t<139>;
      using trade_id_t            = field_ptr_t<1003>;
      using aggressor_indicator_t = field_ptr_t<1057>;

      execution_report_reader_t() = default;

      fix_istream_t &read(fix_istream_t &fs)
      {
        while (fs.has_next())
        {
          auto const field = fs.get_current();
          switch (std::get<0>(field))
          {
          case clord_id_t::get_id():
            clord_id.set_pointers(field);
            break;
          case order_id_t::get_id():
            order_id.set_pointers(field);
            break;
          case symbol_t::get_id():
            symbol.set_pointers(field);
            break;
          case side_t::get_id():
            side.set_pointers(field);
            break;
          case last_shares_t::get_id():
            last_shares.set_pointers(field);
            break;
          case price_t::get_id():
            price.set_pointers(field);
            break;
          case order_qty_t::get_id():
            order_qty.set_pointers(field);
            break;
          case cash_order_qty_t::get_id():
            cash_order_qty.set_pointers(field);
            break;
          case transact_time_t::get_id():
            transact_time.set_pointers(field);
            break;
          case exec_type_t::get_id():
            exec_type.set_pointers(field);
            break;
          case ord_status_t::get_id():
            ord_status.set_pointers(field);
            break;
          case ord_rej_reason_t::get_id():
            ord_rej_reason.set_pointers(field);
            break;
          case no_misc_fees_t::get_id():
            no_misc_fees.set_pointers(field);
            break;
          case misc_fee_amt_t::get_id():
            misc_fee_amt.set_pointers(field);
            break;
          case misc_fee_type_t::get_id():
            misc_fee_type.set_pointers(field);
            break;
          case trade_id_t::get_id():
            trade_id.set_pointers(field);
            break;
          case aggressor_indicator_t::get_id():
            aggressor_indicator.set_pointers(field);
            break;
          default:
            return fs;
          }
        }

        return fs;
      }

      void reset()
      {
        clord_id.reset();
        order_id.reset();
        symbol.reset();
        side.reset();
        last_shares.reset();
        price.reset();
        order_qty.reset();
        cash_order_qty.reset();
        transact_time.reset();
        exec_type.reset();
        ord_status.reset();
        ord_rej_reason.reset();
        no_misc_fees.reset();
        misc_fee_type.reset();
        trade_id.reset();
        aggressor_indicator.reset();
      }

      clord_id_t clord_id{};
      order_id_t order_id{};
      symbol_t   symbol{};

      // 1: buy
      // 2: sell
      side_t           side{};
      last_shares_t    last_shares{};
      price_t          price{};
      order_qty_t      order_qty{};
      cash_order_qty_t cash_order_qty{};
      transact_time_t  transact_time{};
      exec_type_t      exec_type{};
      ord_status_t     ord_status{};

      // 3: insufficent funds
      // 8: post only
      // 0: unknown error
      ord_rej_reason_t ord_rej_reason{};
      no_misc_fees_t   no_misc_fees{};
      misc_fee_amt_t   misc_fee_amt{};
      misc_fee_type_t  misc_fee_type{};
      trade_id_t       trade_id{};
      // Y: for taker orders
      // N: for maker orders
      aggressor_indicator_t aggressor_indicator{};
    };

    struct heartbeat_reader_t
    {
      using test_req_id_t = field_ptr_t<112>;

      heartbeat_reader_t() = default;

      fix_istream_t &read(fix_istream_t &fs)
      {
        while (fs.has_next())
        {
          auto const field = fs.get_current();
          switch (std::get<0>(field))
          {
          case test_req_id_t::get_id():
            testReqId.set_pointers(field);
            break;
          default:
            return fs;
          }
        }

        return fs;
      }

      void reset() { testReqId.reset(); }

      test_req_id_t testReqId;
    };
  }
}
