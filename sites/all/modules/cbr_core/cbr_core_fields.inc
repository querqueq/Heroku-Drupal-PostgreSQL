<?php

/** Field API Hooks * */
function cbr_core_field_info() {
    return array(
        'cbr_number' => array(
            'label' => t('CBR Number'),
            'description' => t('Number field for CBR'),
            'instance_settings' => array('min' => '', 'max' => '', 'prefix' => '', 'suffix' => ''),
            'default_widget' => 'number',
            'default_formatter' => 'number_decimal',
        ),
		'cbr_computed' => array(
      		'label' => t('CBR Computed'),
      		'description' => t('Create field data via PHP code.'),
      		'settings' => array(
        		'code' => '$entity_field[0][\'value\'] = "";',
        		'display_format' => '$display_output = $entity_field_item[\'value\'];',
        		'store' => 1,
        		'recalculate' => FALSE,
        		'database' => array(
				'data_type' => 'varchar',
				'data_length' => 32,
				'data_size' => 'normal',
				'data_precision' => 10,
				'data_scale' => 2,
				'data_not_NULL' => FALSE,
				'data_default' => NULL,
				'data_index' => FALSE,
        		),
      		),
      		'default_widget' => 'computed',
      		'default_formatter' => 'computed_field_plain',
		  	// If we followed the core convention of separate fields for each data
			// type we could make Entity API happy by just setting a property_type.
			// Instead we have to use our own callback to determine the type then
			// rerun theirs to setup the rest of the field properties.
		  	'property_callbacks' => array('computed_field_entity_property_callback'),
		),
        'cbr_range' => array(
            'label' => t('CBR Range'),
            'description' => t('Range field for CBR (floating point format)'),
            'instance_settings' => array(
                'min' => '',
                'max' => '',
                'field' => array(
                    'prefix' => '',
                    'suffix' => '',
                ),
                'from' => array(
                    'label' => t('From'),
                    'prefix' => '',
                    'suffix' => '',
                ),
                'to' => array(
                    'label' => t('to'),
                    'prefix' => '',
                    'suffix' => '',
                ),
                'combined' => array(
                    'prefix' => '',
                    'suffix' => '',
                ),
            ),
            'default_widget' => 'range',
            'default_formatter' => 'range_decimal',
        ),
        'cbr_select' => array(
            'label' => t('CBR Select'),
            'description' => t('Drop down field for CBR.'),
            'settings' => array('allowed_values' => array(), 'allowed_values_function' => ''),
            'default_widget' => 'options_select',
            'default_formatter' => 'list_default',
        ),
        'cbr_taxonomy' => array(
            'label' => t('CBR Term Reference'),
            'description' => t('A reference to a taxonomy term for CBR'),
            'default_widget' => 'options_select',
            'default_formatter' => 'taxonomy_term_reference_link',
            'cardinality' => 6,
            'settings' => array(
                'allowed_values' => array(
                    array(
                        'vocabulary' => '',
                        'parent' => '0',
                    ),
                ),
            ),
        ),
/*        'cbr_core_entityreference' =>  array(
    		'label' => t('CBR Entity Reference'),
    		'description' => t('This field references another entity and supports CBR'),
    		'settings' => array(
      			// Default to the core target entity type node.
      			'target_type' => 'node',
      			// The handler for this field.
      			'handler' => 'base',
      			// The handler settings.
      		'handler_settings' => array(),
    		),
    		'instance_settings' => array(),
    		'default_widget' => 'entityreference_autocomplete',
    		'default_formatter' => 'entityreference_label',
    		'property_callbacks' => array('entityreference_field_property_callback'),
  		) */
    );
}

function cbr_core_field_widget_info_alter(&$info) {
    // Let a new field type re-use an existing widget.
    $info['number']['field types'][] = 'cbr_number';
    $info['range']['field types'][] = 'cbr_range';
    $info['computed']['field types'][] = 'cbr_computed';
    $info['options_select']['field types'][] = 'cbr_select';
    $info['options_select']['field types'][] = 'cbr_taxonomy';
}

/**
 * Implements hook_options_list().
 * Uses list_allowed_values from list.module for CBR Select Flield and a callback function like in taxonomy.module for CBR Taxonomy Field
 */
function cbr_core_options_list($field, $instance, $entity_type, $entity) {
    switch ($field['type']) {
        case 'cbr_select':
            return list_allowed_values($field, $instance, $entity_type, $entity);
        case 'cbr_taxonomy':
            $function = !empty($field['settings']['options_list_callback']) ? $field['settings']['options_list_callback'] : 'taxonomy_allowed_values';
            return $function($field);
    }
}

function cbr_core_field_widget_form_alter(&$element, &$form_state, $context) {
    if ($context['field']['type'] == 'cbr_number') {
        $element['value']['#number_type'] = 'float';
    } else if ($context['field']['type'] == 'cbr_range') {
        $element['#range_type'] = 'float';
    }
}

function cbr_core_field_is_empty($item, $field) {
    switch ($field['type']) {
        case 'cbr_number':
            return (empty($item['value']) && (string) $item['value'] !== '0');
      	case 'cbr_computed':
     		return computed_field_field_is_empty($item, $field);
        case 'cbr_range':
            return (empty($item['from']) && (string) $item['from'] !== '0' && empty($item['to']) && (string) $item['to'] !== '0');
        case 'cbr_select':
            return (empty($item['value']) && (string) $item['value'] !== '0');
        case 'cbr_taxonomy':
            return !is_array($item) || (empty($item['tid']) && (string) $item['tid'] !== '0');
    }
}

/**
 * Implements hook_field_instance_settings_form().
 * Uses _range_field_instance_subelements_settings_form from range.module
 */
function cbr_core_field_instance_settings_form($field, $instance) {
    $settings = $instance['settings'];
    $form = array();
    switch ($field['type']) {
        case 'cbr_number':
            //From number.module
            $form['min'] = array(
                '#type' => 'textfield',
                '#title' => t('Minimum'),
                '#default_value' => $settings['min'],
                '#description' => t('The minimum value that should be allowed in this field. Leave blank for no minimum.'),
                '#element_validate' => array('element_validate_number'),
            );
            $form['max'] = array(
                '#type' => 'textfield',
                '#title' => t('Maximum'),
                '#default_value' => $settings['max'],
                '#description' => t('The maximum value that should be allowed in this field. Leave blank for no maximum.'),
                '#element_validate' => array('element_validate_number'),
            );
            $form['prefix'] = array(
                '#type' => 'textfield',
                '#title' => t('Prefix'),
                '#default_value' => $settings['prefix'],
                '#size' => 60,
                '#description' => t("Define a string that should be prefixed to the value, like '$ ' or '&euro; '. Leave blank for none. Separate singular and plural values with a pipe ('pound|pounds')."),
            );
            $form['suffix'] = array(
                '#type' => 'textfield',
                '#title' => t('Suffix'),
                '#default_value' => $settings['suffix'],
                '#size' => 60,
                '#description' => t("Define a string that should be suffixed to the value, like ' m', ' kb/s'. Leave blank for none. Separate singular and plural values with a pipe ('pound|pounds')."),
            );
            break;
        case 'cbr_range':
            //From range.module
            $form['min'] = array(
                '#type' => 'textfield',
                '#title' => t('Minimum'),
                '#default_value' => $settings['min'],
                '#description' => t('The minimum value that should be allowed in this field. Leave blank for no minimum.'),
                '#element_validate' => array('element_validate_number'),
            );
            $form['max'] = array(
                '#type' => 'textfield',
                '#title' => t('Maximum'),
                '#default_value' => $settings['max'],
                '#description' => t('The maximum value that should be allowed in this field. Leave blank for no maximum.'),
                '#element_validate' => array('element_validate_number'),
            );
            _range_field_instance_subelements_settings_form($form, 'from', $settings);
            _range_field_instance_subelements_settings_form($form, 'to', $settings);
            _range_field_instance_subelements_settings_form($form, 'field', $settings);
            _range_field_instance_subelements_settings_form($form, 'combined', $settings);
            break;
    }
    return $form;
}

function cbr_core_field_settings_form($field, $instance, $has_data) {
    $settings = $field['settings'];
    $form = array();
    if($field['type'] == 'cbr_computed') {
    	$form = computed_field_field_settings_form($field, $instance, $has_data); }
    if ($field['type'] == 'cbr_number' || $field['type'] == 'cbr_core_range') {
        //From number.module
        $form['decimal_separator'] = array(
            '#type' => 'select',
            '#title' => t('Decimal marker'),
            '#options' => array('.' => t('Decimal point'), ',' => t('Comma')),
            '#default_value' => array_key_exists('decimal_separator', $settings) ? $settings['decimal_separator'] : '.',
            '#description' => t('The character users will input to mark the decimal point in forms.'),
        );
    } else if ($field['type'] == 'cbr_select') {
        //From list.module:
        $form['allowed_values'] = array(
            '#type' => 'textarea',
            '#title' => t('Allowed values list'),
            '#default_value' => list_allowed_values_string($settings['allowed_values']),
            '#rows' => 10,
            '#element_validate' => array('cbr_core_list_allowed_values_setting_validate'),
            '#field_has_data' => $has_data,
            '#field' => $field,
            '#field_type' => $field['type'],
            '#access' => empty($settings['allowed_values_function']),
        );
        $description = '<p>' . t('The possible values this field can contain. Enter one value per line, in the format key|label.');
        $description .= '<br/>' . t('The key is the stored value. The label will be used in displayed values and edit forms.');
        $description .= '</p>';
        $form['allowed_values']['#description'] = $description;
    } else if ($field['type'] == 'cbr_taxonomy') {
        //From taxonomy.module
        // Get proper values for 'allowed_values_function', which is a core setting.
        $vocabularies = taxonomy_get_vocabularies();
        $options = array();
        foreach ($vocabularies as $vocabulary) {
            $options[$vocabulary->machine_name] = $vocabulary->name;
        }
        $form['allowed_values'] = array(
            '#tree' => TRUE,
        );

        foreach ($field['settings']['allowed_values'] as $delta => $tree) {
            $form['allowed_values'][$delta]['vocabulary'] = array(
                '#type' => 'select',
                '#title' => t('Vocabulary'),
                '#default_value' => $tree['vocabulary'],
                '#options' => $options,
                '#required' => TRUE,
                '#description' => t('The vocabulary which supplies the options for this field.'),
                '#disabled' => $has_data,
            );
            $form['allowed_values'][$delta]['parent'] = array(
                '#type' => 'value',
                '#value' => $tree['parent'],
            );
        }
    }
    return $form;
}

/**
 * Injects the Field CBR settings on the Edit field tab.
 * Implements of hook_form_FORM_ID_alter().
 */
function cbr_core_form_field_ui_field_edit_form_alter(&$form, &$form_state, $form_id) {
    //Insert CBR Field Settings
    _cbr_core_get_field_settings_form($form, $form['#field']['type']);

    //Set cardinality to 1 for our cbr fields
    if (in_array($form['#field']['type'], array('cbr_core_range', 'cbr_number', 'cbr_select', 'cbr_taxonomy'))) {
        $form['field']['cardinality']['#options'] = array(1 => 1);
    }
}

/**
 * Injects the Field CBR settings on the field settings tab.
 * Implements of hook_form_FORM_ID_alter().
 */
function cbr_core_form_field_ui_field_settings_form_alter(&$form, &$form_state, $form_id) {
    //Insert CBR Field Settings
    _cbr_core_get_field_settings_form($form, $form['field']['type']['#value']);
}

function _cbr_core_get_field_settings_form(&$form, $field_type) {
    $registered_fields = variable_get('cbr_core_registered_fields', array());
    if (in_array($field_type, $registered_fields)) {
        $form['field']['cbr_settings'] = array(
            '#weight' => -10,
        );
        $form['field']['cbr_settings']['enabled'] = array(
            '#title' => t('Case Based Reasoning enabled'),
            '#type' => 'checkbox',
            '#default_value' => isset($form['#field']['cbr_settings']['enabled']) ? $form['#field']['cbr_settings']['enabled'] : cbr_core_is_field_default_enabled($field_type),
        );
        $form['field']['cbr_settings']['weight'] = array(
            '#type' => 'textfield',
            '#title' => t('Attribute Weight'),
            '#default_value' => isset($form['#field']['cbr_settings']['weight']) ? $form['#field']['cbr_settings']['weight'] : 1,
            '#required' => FALSE,
            '#element_validate' => array('element_validate_number'),
            '#description' => t('Attribute Weight for CBR calculation. Can be changed, but you need to recalulate all saved similarity factors!'),
        );
        $form['field']['cbr_settings']['max_distance_value'] = array(
            '#type' => 'textfield',
            '#title' => t('Maximal distance of values'),
            '#default_value' => isset($form['#field']['cbr_settings']['max_distance']) ? $form['#field']['cbr_settings']['max_distance'] : 1000,
            '#required' => FALSE,
            '#element_validate' => array('element_validate_number'),
            '#description' => t('Attribute maximal distance between two values of this field. Can be changed, but you need to recalulate all saved similarity factors!'),
        );
        $form['#submit'][] = 'cbr_core_get_field_settings_form_submit';
    }
    return $form;
}

function cbr_core_is_field_default_enabled($field_type) {
    return in_array($field_type, array('cbr_range', 'cbr_number', 'cbr_select', 'cbr_taxonomy'));
}

function cbr_core_get_field_settings_form_submit(&$form, &$form_state) {
    $cbr_enabled = $form_state['input']['field']['cbr_settings']['enabled'];
    if ($cbr_enabled == 1) {
        drupal_set_message(t('Case Based Reasoning is active on this field! If you use this field on multiple node types, it`s active on each of them. To turn CBR off, go back and uncheck "Case Based Reasoning enabled".'));
    } else {
        drupal_set_message(t('Case Based Reasoning is inactive on this field! To turn CBR on, go back and check "Case Based Reasoning enabled".'), 'warning');
    }
}

function cbr_core_field_formatter_info_alter(&$info) {
    // Let a new field type re-use an existing formatter.
    $info['number_decimal']['field types'][] = 'cbr_number';
    $info['range_decimal']['field types'][] = 'cbr_range';
    $info['list_default']['field types'][] = 'cbr_select';
    $info['taxonomy_term_reference_link']['field types'][] = 'cbr_taxonomy';
}

/** Validation * */

/**
 * Element validate callback; check that the entered values are valid.
 */
function cbr_core_list_allowed_values_setting_validate($element, &$form_state) {
    $field = $element['#field'];
    $has_data = $element['#field_has_data'];
    $field_type = $field['type'];
    $values = list_extract_allowed_values($element['#value'], $field['type'], FALSE);

    if (!is_array($values)) {
        form_error($element, t('Allowed values list: invalid input.'));
    } else {
        // Check that keys are valid for the field type.
        foreach ($values as $key => $value) {
            if (!is_numeric($key)) {
                form_error($element, t('Allowed values list: each key must be a valid integer or decimal.'));
                break;
            }
        }
        // Prevent removing values currently in use.
        if ($has_data) {
            $lost_keys = array_diff(array_keys($field['settings']['allowed_values']), array_keys($values));
            if (_list_values_in_use($field, $lost_keys)) {
                form_error($element, t('Allowed values list: some values are being removed while currently in use.'));
            }
        }
        form_set_value($element, $values, $form_state);
    }
}
