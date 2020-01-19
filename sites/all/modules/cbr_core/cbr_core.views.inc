<?php

function cbr_core_views_data() {
    $data = array();
    $data['cbr_similarity_factor']['table']['group'] = t('Case Based Reasoning');

    $data['cbr_similarity_factor']['table']['join'] = array(
        'node' => array(
            'left_field' => 'nid',
            'field' => 'nid2',
        ),
    );

    $data['cbr_similarity_factor']['nid1'] = array(
        'title' => t('Base Case Node ID'),
        'help' => t('Base case for which similar cases should be found.'),
        'argument' => array(
            'handler' => 'views_handler_argument_node_nid',
        ),
    );

    $data['cbr_similarity_factor']['similarity_factor'] = array(
        'title' => t('Similarity'),
        'help' => t('Shows how similar a case to another case is. A higher similarity means a more similar case.'),
        'field' => array(
            'handler' => 'views_handler_field_numeric',
            'click sortable' => TRUE,
            'float' => TRUE
        ),
        'filter' => array(
            'handler' => 'views_handler_filter_numeric',
        ),
        'sort' => array(
            'handler' => 'views_handler_sort',
        ),
    );
    return $data;
}

function cbr_core_views_default_views() {
    $views = array();
    $path = drupal_get_path('module', 'cbr_core') . '/views/';

    $files = array(
        'cbr_sample_view.inc',
        'cbr_view.inc'
    );

    foreach ($files as $file) {
        require_once $path . $file;
        $views[$view->name] = $view; // $view is in included file
        unset($view);
    }
    return $views;
}
