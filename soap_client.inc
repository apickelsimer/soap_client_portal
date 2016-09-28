<?php
/**
 * @file
 * Utility functions for running the tester UI for web services.
 */


/**
 * FAPI form used to display the options and the results of calling a web
 * service.
 */
function soap_client_operation_invoke($form, $form_state, $service, $operation) {
  $strings = array(
    '!service_label' => $service->label,
    '!operation_label' => $operation['label'],
  );
  drupal_set_title(t("Testing Web Service : !service_label : !operation_label()", $strings));
  $form  = array();

  $form['header-title'] = array(
  '#markup' => '<h2>' . $operation['label'] . '</h2><br>',
  );

  /* hide this for now
    $form['uri'] = array(
      '#title' => 'Service URI',
      '#type' => 'textfield',
      '#disabled' => TRUE,
      '#value' => $service->url,
    );
  */

  $form['parameters'] = array(
    '#title' => 'Parameters',
    '#type' => 'container',
    '#tree' => TRUE,
    '#value' => t("Enter the parameters to the !operation_label service here. It's up to you to get the data types right. No type validation is done at this end, as it's a debugger to let you throw various errors at the web service and see how it responds.", $strings),
  );

  // Deal with complex types.
  // Each complex type may require its own mini-form for data entry,
  // and these may be nested.
  $datatypes = $service->datatypes;
  foreach ($operation['parameter'] as $param => $info) {
    $form['parameters'][$param] = wsclient_tester_data_entry_form($param, $info['type'], @$info['description'], @$form_state['values']['parameters'], $datatypes);
  }
  $form['execute'] = array(
    '#type' => 'submit',
    '#value' => 'Execute Request',
    '#ajax' => array(
      'event' => 'click',
      'callback' => 'wsclient_tester_prepare_request_callback',
      'wrapper' => 'edit-transaction',
      'method' => 'replace',
      'effect' => 'fade',
    ),
  );

  //add some markup here
  $form['spacer'] = array(
  '#markup' => '<br/><br/>',
  );

  // Set up result panes. Content for these usually gets filled in via ajax.

  $form['transaction'] = array(
    '#type' => 'fieldset',
    '#title' => 'Transaction',
    '#attributes' => array('id' => 'edit-transaction'),
  );

  $form['transaction']['request'] = array(
    '#type' => 'fieldset',
    '#title' => 'Request',
  );

  $form['transaction']['request']['header'] = array(
    '#markup' => 'Headers go here',
    '#prefix' => '<pre>',
    '#suffix' => '</pre>',
  );

  $form['transaction']['request']['packet'] = array(
    '#markup' => 'Packet goes here',
    '#prefix' => '<pre>',
    '#suffix' => '</pre>',
  );

  $form['transaction']['request']['data'] = array();

  $form['transaction']['response'] = array(
    '#type' => 'fieldset',
    '#title' => 'Response',
  );
  $form['transaction']['response']['header'] = array(
    '#markup' => 'ResponseHeaders go here',
    '#prefix' => '<pre>',
    '#suffix' => '</pre>',
  );

  $form['transaction']['response']['packet'] = array(
    '#markup' => 'ResponsePayload goes here',
    '#prefix' => '<pre>',
    '#suffix' => '</pre>',
  );

  $form['transaction']['response']['data'] = array();

  return $form;
}

/**
 * FAPI form used to display all operations of a service
 */
function soap_client_operation_viewer($form, $form_state, $service) {
  $strings = array(
    '!service_label' => $service->label,
  );
  drupal_set_title(t("Testing Web Service : !service_label", $strings));

  $type_info = wsclient_get_types();
    if (empty($type_info)) {
      drupal_set_message(t('No service types were found, please enable a module that provides a service type.'), 'warning');
    }
    $types = array();
    foreach ($type_info as $type => $info) {
      $types[$type] = $info['label'];
    }

    /* hide this for now
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => 'Service Name:',
      '#default_value' => $service->label,
      '#disabled' => TRUE,
      '#weight' => 10,
    );
    */

      $form['header-title'] = array(
        '#markup' => '<h2>' . $service->label . '</h2><br>',
        );

    /* hide this for now
    $form['url'] = array(
      '#type' => 'textfield',
      '#title' => 'URL',
      '#default_value' => $service->url,
      '#disabled' => TRUE,
      '#element_validate' => array('wsclient_ui_element_url_validate'),
      '#weight' => 30,
    );
    */

   $rows = array();
      $operations = soap_client_label_sort($service->operations);
      foreach ($operations as $name => $operation) {
        $row = array();
        $row[] = array('data' => array(
          '#theme' => 'entity_ui_overview_item',
          '#label' => $operation['label'],
          '#name' => $name,
          '#url' => array(
            'path' => '/soap/' . $service->name . '/operation/' . $name . '/invoke',
            'options' => array(),
          ),
        ));

        $rows[] = $row;
      }

      $row = array();
      // @todo description help text for operations, data types
      $form['operations'] = array(
        '#access' => TRUE,
        '#tree' => TRUE,
        '#theme' => 'table',
        '#empty' => t('None'),
        '#caption' => t('Operations'),
        '#rows' => $rows,
        '#header' => $header,
        '#weight' => 50,
      );
      // Add some table styling from Rules.
      $form['operations']['#attributes']['class'][] = 'table table-striped';
      $form['operations']['#attached']['css'][] = drupal_get_path('module', 'soap_client') . '/ui/ui.css';



      // Data types of the web service in a table
      $rows = array();
      $datatypes = soap_client_label_sort($service->datatypes);
      foreach ($datatypes as $name => $datatype) {
        $row = array();
        $row[] = array('data' => array(
          '#theme' => 'entity_ui_overview_item',
          '#label' => $datatype['label'],
          '#name' => $name,
          '#url' => array(
            'path' => '#',
            'options' => array(),
          ),
        ));
        $rows[] = $row;
      }

      $row = array();

      $form['datatypes'] = array(
        '#access' => TRUE,
        '#tree' => TRUE,
        '#theme' => 'table',
        '#empty' => t('None'),
        '#caption' => t('Data types'),
        '#rows' => $rows,
        '#header' => $header,
        '#weight' => 60,
      );
      $form['datatypes']['#attributes']['class'][] = 'table table-striped';
      $form['datatypes']['#attached']['css'][] = drupal_get_path('module', 'soap_client') . '/ui/ui.css';

    return $form;
}

/**
 * FAPI form used to display all services that are managed
 */
function soap_client_services_viewer($form, $form_state, $service) {
  drupal_set_title(t("Web Services"));
    //to-do Pull from Entity to list services
    //Here we will list all of the available services

    return $form;
}

/**
 * Callback for the test from 'prepare' button.
 */
function wsclient_tester_prepare_request_callback($form, $form_state) {
  $service = $form_state['build_info']['args'][0];
  $operation = $form_state['build_info']['args'][1];

  // Convert the form values into a data structure suitable for making the query.
  // Magic?
  // service::invoke will cast this array into the correct paramaterized objects
  // According to the $operation['parameter'] schema. Nifty.
  $args = array();
  if (!empty($form_state['values']['parameters'])) {
    $args = $form_state['values']['parameters'];
  }

  if ($service->type == 'soap') {
    // The service will have an endpoint that will have a SOAPClient.
    // Settings on the service->options may be passed to the SOAPClient.
    // @see WSClientSOAPEndpoint::client()
    // @see http://www.php.net/manual/en/soapclient.soapclient.php
    $service->settings['options']['trace'] = TRUE;
    // disable cache when testing!
    $service->settings['options']['cache_wsdl'] = WSDL_CACHE_NONE;
  }

  // Ready to actually invoke the call
  $timer_before = microtime();
  try {
    $response = $service->invoke($operation['name'], $args);
  }
  catch (Exception $e) {
    $response = $e->getMessage();
  }
  $timer_duration = $timer_before - microtime();

  $element = $form['transaction'];

  // Place the trace data into the display.
  if ($service->type == 'rest') {
    $operation = $service->operations[$operation['name']];
    $parameters = '';
    foreach ($args as $para_name => $para_value) {
      $parameters .= $para_name . '=' . $para_value . '<br>';
    }
    $reqInfo = 'Uri:' . $service->url . $operation['url'] . '<br>' .
      'Method:' . (isset($operation['type']) ? $operation['type'] : 'GET') . '<br>' .
      'Parameter:' . $parameters;
    $element['request']['packet']['#markup'] = $reqInfo;
    $element['response']['header']['#markup'] = $service->endpoint()->client()->lastResponse->headers;
    $element['response']['packet']['#markup'] = $service->endpoint()->client()->lastResponse->body;
  }
  elseif ($service->type == 'soap') {
    $element['request']['header']['#markup'] = $service->endpoint()->client()->__getLastRequestHeaders();
    $element['request']['packet']['#markup'] = htmlspecialchars(wsclient_tester_prettify_xml($service->endpoint()->client()->__getLastRequest()));
    $element['response']['header']['#markup'] = $service->endpoint()->client()->__getLastResponseHeaders();
    $element['response']['packet']['#markup'] = htmlspecialchars(wsclient_tester_prettify_xml($service->endpoint()->client()->__getLastResponse()));
  }

  /* hide this for now
  if (module_exists('devel')) {
    $element['request']['data']['#markup'] = kpr($args, 1);
    $element['response']['data']['#markup'] = kpr($response, 1);
   }
   */

  $element['#value'] = t("Ran at %time, took %duration to execute", array(
    '%time' => time(),
    '%duration' => $timer_duration,
  ));

  return $element;
}

/**
 * Indent and format XML for display.
 */
function wsclient_tester_prettify_xml($xml) {
  $dom = new DOMDocument;
  $dom->preserveWhiteSpace = FALSE;
  $dom->loadXML($xml);
  $dom->formatOutput = TRUE;
  return $dom->saveXml();
}

/**
 * Helper function to sort a nested data information array based on the label
 * of the items.
 */
function soap_client_label_sort($data_info, $label_key = 'label') {
  $sort_info = array();
  foreach ($data_info as $key => $info) {
    $sort_info[$key] = $info[$label_key];
  }
  natcasesort($sort_info);
  foreach ($sort_info as $key => $label) {
    $sort_info[$key] = $data_info[$key];
  }
  return $sort_info;
}

/**
 * A mini form element representing the given data type.
 * textfield for most things, but nested fieldsets for complex types.
 *
 * This recurses through complex types until it hits core types.
 *
 * @return a FAPI form fragment.
 */
function wsclient_tester_data_entry_form($label, $type, $description, $data, $datatypes) {
  if (isset($datatypes[$type])) {
    // Build a complex type.
    $datatype = $datatypes[$type];
    $element = array(
      '#type' => 'fieldset',
      '#title' => check_plain("$label ({$datatype['label']})"),
      '#collapsible' => TRUE,
    );
    foreach ($datatype['property info'] as $field_id => $field_info) {
      // Recurse and get each bit to render its own input element.
      $element[$field_id] = wsclient_tester_data_entry_form($field_id, $field_info['type'], @$field_info['description'], $data[$field_id], $datatypes);
    }
    return $element;
  }
  elseif (preg_match('/^list\<(.*)\>$/', $type, $matches)) {
    // Strange notation, type="list<MyType>" means a list of those things.
    // @see wsclient_soap_type_mapper()
    // This becomes a numerically indexed array.
    // Present it in the form as a nested list.
    $actual_type = $matches[1];
    $element = array(
      '#type' => 'fieldset',
      '#title' => t("List of %label (%type)", array('%label' => $label, '%type' => $type)),
      '#collapsible' => TRUE,
    );
    for ($field_id = 0; $field_id < 3; $field_id++) {
      // Recurse and get each bit to render its own input element
      $element[$field_id] = wsclient_tester_data_entry_form($field_id, $actual_type, NULL, $data[$field_id], $datatypes);
      $element[$field_id]['#collapsed'] = TRUE;
    }
    return $element;
  }
  elseif ($type == 'text_formatted') {
    return array(
      '#type' => 'textarea',
      '#title' => t("%label (%type)", array('%label' => $label, '%type' => $type)),
      '#default_value' => $data,
      '#description' => $description,
    );
  }
  elseif ($type == 'boolean') {
    return array(
      '#type' => 'select',
      '#title' => t("%label (%type)", array('%label' => $label, '%type' => $type)),
      '#default_value' => $data,
      // Defining false first so it's the default when setting up a form.
      '#options' => array('false' => 'false', 'true' => 'true'),
      '#description' => $description,
    );
  }
  else {
    // A textfield will normally do for any other primitives.
    return array(
      '#type' => 'textfield',
      '#title' => t("%label (%type)", array('%label' => $label, '%type' => $type)),
      '#default_value' => $data,
      '#size' => 20,
      '#description' => $description,
    );
  }
}
