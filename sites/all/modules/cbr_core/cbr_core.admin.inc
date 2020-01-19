<?php

module_load_include('inc', 'cbr_core', 'cbr_core_calculate');

function cbr_core_admin_settings() {
    $form = array();
	$form['calculation'] = array(
		'#type' => 'fieldset', 
		'#title' => t('Calculation Process'),
		'#collapsible' => TRUE,
		'#collapsed' => TRUE,
		);
    $form['calculation']['select_recalc'] = array(
        '#type' => 'select',
        '#options' => cbr_core_get_active_node_types(),
        '#title' => t('Recalculate Similarity'),
        '#description' => t('Sometimes you need to recalculate the similarities of a content type. To do so, please select a content type and click "Recalculate". Only content types with one or more "Registered CBR fields" are selectable.'),
        '#default_value' => '-',
    );
    $form['calculation']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Recalculate'),
        '#submit' => array('cbr_core_admin_calculation_submit'),
    );
    //$form['#submit'][] = 'cbr_core_admin_settings_submit';
	$form['display'] = array(
		'#type' => 'fieldset', 
		'#title' => t('Displaying Cases'),
		'#collapsible' => TRUE,
		'#collapsed' => FALSE,
		);
    $form['display']['number_of_similar_cases to_be_shown'] = array(
		'#type' => 'textfield',
		'#title' => t('Num Similar Cases'),
		'#size' => 5, 
		'#required' => true,
		'#default_value' => variable_get('cbr_num_similar_cases', 10),
		// add validation
		'#extra_validate' => array('type' => 'integer', 'min' => 3, 'max' => 99),
		'#description' => t('How many similar cases shall be shown'),
		);
    $form['display']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#submit' => array('cbr_core_admin_display_submit'),
    );
	$form['calculation_process'] = array(
		'#type' => 'fieldset', 
		'#title' => t('Calculation Process'),
		'#collapsible' => TRUE,
		'#collapsed' => FALSE,
		);
    $form['calculation_process']['cbr_automated_calculation'] = array(
		'#type' => 'checkbox',
		'#title' => t('Automated Calculation'),
		'#default_value' => variable_get('cbr_automated_calculation', TRUE),
		'#description' => t('Shall the calculation started automatically if a case is changed'),
	);
    $form['calculation_process']['cbr_build_similarity_structure'] = array(
		'#type' => 'checkbox',
		'#title' => t('Build an intermediate structure during calculation of similarity'),
		'#default_value' => variable_get('cbr_build_similarity_structure', TRUE),
		'#description' => t('If true computation is faster but requires more memory. If two many cases must be compared the option should be unchecked'),
		);
    $form['calculation_process']['cbr_inverse_similarity'] = array(
		'#type' => 'checkbox',
		'#title' => t('Inverse similarity'),
		'#default_value' => variable_get('cbr_inverse_similarity', TRUE),
		'#description' => t('If similarity is an inverse relation we have to calculate fewer relations'),
		);
    $form['calculation_process']['cbr_max_distance'] = array(
		'#type' => 'textfield',
		'#title' => t('Maximal distance in Taxonomies'),
		'#size' => 5, 
		'#required' => true,
		'#default_value' => variable_get('cbr_max_distance', TRUE),
		'#description' => t('Maximal distance in Taxonomies'),
		);
    $form['calculation_process']['cbr_detailed_results'] = array(
		'#type' => 'checkbox',
		'#title' => t('Detailed results'),
		'#default_value' => variable_get('cbr_detailed_results', TRUE),
		'#description' => t('Show more detailed results in the process of calculation of simularity'),
		);
    $form['calculation_process']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#submit' => array('cbr_core_admin_calculation_process_submit'),
    );


	$form['fields'] = array(
		'#type' => 'fieldset', 
		'#title' => t('Registered CBR Fields'),
		'#collapsible' => TRUE,
		'#collapsed' => FALSE,
		);
    $registered_fields = variable_get('cbr_core_registered_fields', array());
    $ul = '<ul>';
    foreach ($registered_fields as $field) {
        $ul .= '<li>' . $field . '</li>';
    }
    $ul .= "</ul>";
    $form['fields']['registered_fields'] = array('#type' => 'item',
        '#title' => t('Registered CBR fields'),
        '#description' => t('This fields have case based reasoning functionality: ' . $ul),
    );
    return $form;
}

function cbr_core_admin_get_started() {
    global $base_url;
    $form = array();
    $form['getting_started'] = array('#type' => 'item',
        '#title' => t('Getting Started!'),
        '#description' => t('<br>Define the fields of a case at <a href = "'.$base_url. '/admin/structure/types/manage/case/fields">field definition</a>.
        	<br><br>If you enable the module <font color="red">CBR Example</font>, a case definition with 7 fields is installed and some cases are loaded automatically.
        	<br><br><a href = "' . $base_url . '/node/add/case">Create one or more Cases</a>. All existing cases can be found <a href = "' . $base_url . '/cbr">here</a>.<br><br>'
                . '<a href = "' . $base_url . '/admin/structure/block">Move the Block</a> "View:  CBR Similar Cases View" to any region you like (usually to the content region). This block displays similar cases when enabled on a content type with one or more cbr fields. You can alter the <a href = "' . $base_url . '/admin/structure/views/view/cbr_similar_cases_view/edit">view</a>'
                . ' of this block and add additional fields to it or create your own view using the field "Similarity" from the categorie "Case-based Reasoning" <br><br>'
                . 'You can create a <a href = "' . $base_url . '/admin/structure/types/add">new content typ</a> and add one or more <a href = "' . $base_url . '/admin/structure/types">fields</a> of type CBR Computed, CBR Number, CBR Range, CBR Select, CBR Term Reference, EntityReference or another type listed at "Registered CBR fields" to enable CBR functionality on this content type. <br><br> '
                . 'For detailed information about the functionality of the module and the  <a href = "' . $base_url . '/admin/config/cbr/doc#calculation-of-similarity">calculation of the similarty</a> please read the <a href = "' . $base_url . '/admin/config/cbr/doc">documentation</a>.<br><br>'
                . 'This module has an api. For more information visit the <a href = "' . $base_url . '/admin/config/cbr/doc/#api-reference">api reference</a>. For examples of its hooks open "cbr_core.api.php" in the module`s folder.'),
    );
    return $form;
}

function cbr_core_admin_documentation() {
    drupal_goto(drupal_get_path('module', 'cbr_core') . "/doc/doc.html");
}

function cbr_core_get_active_node_types() {
    $active_node_types = array();
    $active_node_types['-'] = t('None');

    $all_nodetypes = node_type_get_names();
    foreach ($all_nodetypes as $node_type => $type_name) {
        if (cbr_core_is_enabled_on_content_type($node_type)) {
            $active_node_types[$node_type] = $type_name;
        }
    }
    return $active_node_types;
}

function cbr_core_admin_calculation_submit($form, &$form_state) {
    $type = $form_state['values']['select_recalc'];
    $node_title = $form_state['values']['node_title'];
    if ($type == '-' && !isset($node_title)) {
        drupal_set_message(t('Please select a content type or a title'));
    } else if(isset($node_title)&&strlen($node_title))
    	{
    		$nodes = node_load_multiple(array(), array('title' => $node_title));
    		foreach($nodes as $node) {
    			cbr_core_calculate_similarity($node->type, $node->nid);
    		}
    	}
    	else {
        cbr_core_calculate_similarity($type);
    }
}
function cbr_core_admin_display_submit($form, &$form_state) {
	$ac = variable_set('cbr_num_similar_cases', ($form_state['values']['display']['number_of_similar_cases to_be_shown']));
}
function cbr_core_admin_calculation_process_submit($form, &$form_state) {
	$ac = variable_set('cbr_automated_calculation', ($form_state['values']['cbr_automated_calculation']));
	$bc = variable_set('cbr_build_similarity_structure',     ($form_state['values']['cbr_build_similarity_structure']));
	$cc = variable_set('cbr_inverse_similarity',     ($form_state['values']['cbr_inverse_similarity']));
	$dc = variable_set('cbr_max_distance',     ($form_state['values']['cbr_max_distance']));
	$ec = variable_set('cbr_detailed_results',     ($form_state['values']['cbr_detailed_results']));

}
function cbr_core_admin_goto_settings() {
    drupal_goto("admin/config/cbr");
}
