<?php
$view = new view();
$view->name = 'cbr_similar_cases_view';
$view->description = 'This view contains a block. Activate the block and display it on a page of a case to get similar cases.';
$view->tag = 'default';
$view->base_table = 'node';
$view->human_name = 'CBR Similar Cases View';
$view->core = 7;
$view->api_version = '3.0';
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

/* Display: Master */
$handler = $view->new_display('default', 'Master', 'default');
$handler->display->display_options['title'] = 'Similar Cases';
$handler->display->display_options['use_more_always'] = FALSE;
$handler->display->display_options['use_more_text'] = 'mehr';
$handler->display->display_options['access']['type'] = 'perm';
$handler->display->display_options['cache']['type'] = 'none';
$handler->display->display_options['query']['type'] = 'views_query';
$handler->display->display_options['exposed_form']['type'] = 'basic';
$handler->display->display_options['pager']['type'] = 'mini';
$handler->display->display_options['pager']['options']['items_per_page'] = '10';
$handler->display->display_options['style_plugin'] = 'table';
$handler->display->display_options['style_options']['columns'] = array(
  'title' => 'title',
  'similarity_factor_1' => 'similarity_factor_1',
);
$handler->display->display_options['style_options']['default'] = 'similarity_factor_1';
$handler->display->display_options['style_options']['info'] = array(
  'title' => array(
    'sortable' => 1,
    'default_sort_order' => 'asc',
    'align' => '',
    'separator' => '',
    'empty_column' => 0,
  ),
  'similarity_factor_1' => array(
    'sortable' => 1,
    'default_sort_order' => 'desc',
    'align' => '',
    'separator' => '',
    'empty_column' => 0,
  ),
);
/* Field: Content: Title */
$handler->display->display_options['fields']['title']['id'] = 'title';
$handler->display->display_options['fields']['title']['table'] = 'node';
$handler->display->display_options['fields']['title']['field'] = 'title';
$handler->display->display_options['fields']['title']['label'] = 'Case Title';
$handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
$handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
/* Field: Content: Budget */
$handler->display->display_options['fields']['field_cbr_case_budget']['id'] = 'field_cbr_case_budget';
$handler->display->display_options['fields']['field_cbr_case_budget']['table'] = 'field_data_field_cbr_case_budget';
$handler->display->display_options['fields']['field_cbr_case_budget']['field'] = 'field_cbr_case_budget';
$handler->display->display_options['fields']['field_cbr_case_budget']['settings'] = array(
  'thousand_separator' => '',
  'decimal_separator' => '.',
  'scale' => '2',
  'prefix_suffix' => 1,
);
/* Field: Content: Size */
$handler->display->display_options['fields']['field_cbr_case_size']['id'] = 'field_size';
$handler->display->display_options['fields']['field_cbr_case_size']['table'] = 'field_data_field_size';
$handler->display->display_options['fields']['field_cbr_case_size']['field'] = 'field_size';
/* Field: Content: Status */
$handler->display->display_options['fields']['field_cbr_case_status']['id'] = 'field_status';
$handler->display->display_options['fields']['field_cbr_case_status']['table'] = 'field_data_field_status';
$handler->display->display_options['fields']['field_cbr_case_status']['field'] = 'field_status';
/* Field: Case Based Reasoning: Similarity */
$handler->display->display_options['fields']['similarity_factor_1']['id'] = 'similarity_factor_1';
$handler->display->display_options['fields']['similarity_factor_1']['table'] = 'cbr_similarity_factor';
$handler->display->display_options['fields']['similarity_factor_1']['field'] = 'similarity_factor';
$handler->display->display_options['fields']['similarity_factor_1']['set_precision'] = TRUE;
$handler->display->display_options['fields']['similarity_factor_1']['precision'] = '4';
$handler->display->display_options['fields']['similarity_factor_1']['separator'] = ' ';
/* Sort criterion: Case Based Reasoning: Similarity */
$handler->display->display_options['sorts']['similarity_factor']['id'] = 'similarity_factor';
$handler->display->display_options['sorts']['similarity_factor']['table'] = 'cbr_similarity_factor';
$handler->display->display_options['sorts']['similarity_factor']['field'] = 'similarity_factor';
$handler->display->display_options['sorts']['similarity_factor']['order'] = 'DESC';
/* Contextual filter: Case Based Reasoning: Base Case Node ID */
$handler->display->display_options['arguments']['nid1']['id'] = 'nid1';
$handler->display->display_options['arguments']['nid1']['table'] = 'cbr_similarity_factor';
$handler->display->display_options['arguments']['nid1']['field'] = 'nid1';
$handler->display->display_options['arguments']['nid1']['default_action'] = 'default';
$handler->display->display_options['arguments']['nid1']['exception']['title'] = 'Alle';
$handler->display->display_options['arguments']['nid1']['default_argument_type'] = 'node';
$handler->display->display_options['arguments']['nid1']['summary']['number_of_records'] = '0';
$handler->display->display_options['arguments']['nid1']['summary']['format'] = 'default_summary';
$handler->display->display_options['arguments']['nid1']['summary_options']['items_per_page'] = '25';
/* Filter criterion: Content: Published */
$handler->display->display_options['filters']['status']['id'] = 'status';
$handler->display->display_options['filters']['status']['table'] = 'node';
$handler->display->display_options['filters']['status']['field'] = 'status';
$handler->display->display_options['filters']['status']['value'] = 1;
$handler->display->display_options['filters']['status']['group'] = 1;
$handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
/* Filter criterion: Case Based Reasoning: Similarity */
$handler->display->display_options['filters']['similarity_factor']['id'] = 'similarity_factor';
$handler->display->display_options['filters']['similarity_factor']['table'] = 'cbr_similarity_factor';
$handler->display->display_options['filters']['similarity_factor']['field'] = 'similarity_factor';
$handler->display->display_options['filters']['similarity_factor']['operator'] = '>';
$handler->display->display_options['filters']['similarity_factor']['value']['value'] = '-1';

/* Display: Block */
$handler = $view->new_display('block', 'Block', 'block_1');
