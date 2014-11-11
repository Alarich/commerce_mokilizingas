<?php
/**
 * Checkout pane callback: presents a completion message on the complete page.
 */
function commerce_mlizingas_pane_checkout_form($form, &$form_state, $checkout_pane, $order) {
    $pane_form = array();

    // show only for mokilizingas WP payments
    if ($order->data['payment_method'] == 'mlizingas_wps|commerce_payment_mlizingas_wps') {
        $message = variable_get('commerce_mlizingas_pane_advance_payment_message', '');
        $leftToPay = commerce_payment_order_balance($order);

        if ($leftToPay['amount'] > 0 ) {
            $message = t($message, array('@amount' => commerce_currency_format($leftToPay['amount'], $leftToPay['currency_code'])));

            $pane_form['advance_payment_message'] = array(
                '#markup' => '<div class="mlizingas_complete_advance_payment_info">' . $message . '</div>',
            );
        }
    }

    return $pane_form;
}

/**
 * Implements base_settings_form()
 */
function commerce_mlizingas_pane_settings_form($checkout_pane) {
    $default = 'You will need to pay @amount on delivery.';

    $form['commerce_mlizingas_pane_advance_payment_message'] = array(
        '#type' => 'textarea',
        '#cols' => 20,
        '#rows' => 5,
        '#title' => t('Advance payment message'),
        '#default_value' => variable_get('commerce_mlizingas_pane_advance_payment_message', $default),
        '#description' => 'Use @amount to display amount which customer needs to pay on delivery.'
    );
    return $form;
}