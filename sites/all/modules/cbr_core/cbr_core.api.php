<?php

/**
 * @file 
 * Hooks provided by the cbr core module.
 */

/**
 * This hook can be used to override the calculation of the similarity between two nodes.  
 * 
 * @param $node1 The first node
 * @param $node2 The second node
 * @param $field_name The name of the field
 * @param $field_type The type of the field
 * @return The calculated similarity 
 */
function hook_calculate_cbr_similarity_per_field($node1, $node2, $field_name, $field_type) {
    //This is an example implemention of the hook
    //The default calculation is replaced by a square division calculation
    //Get the field's language
    $field_language1 = field_language('node', $node1, $field_name);
    $field_language2 = field_language('node', $node2, $field_name);

    //Choose the right field type
    if ($field_type == 'cbr_number') {
        //Use cbr_core_get_field_value from cbr_core_calculate.inc to get the value of a field
        $node1_fieldvalue = cbr_core_get_field_value($node1, $field_name, $field_language1, 'value');
        $node2_fieldvalue = cbr_core_get_field_value($node2, $field_name, $field_language2, 'value');

        //Example calculation: Square Division
        return abs($node1_fieldvalue * $node1_fieldvalue - $node2_fieldvalue * $node2_fieldvalue);
    }
    //Otherwise, return nothing or NULL
}

/**
 *  This hook allows you to alter the batch configuration for calculating and saving the similarities.
 *  You can override the calculation process completely. 
 *  To save the similarity you can use cbr_core_save_similarty from cbr_core_calculate.inc 
 *  For more information about the drupal's batch system visit 
 *  https://api.drupal.org/api/drupal/includes%21form.inc/group/batch/7.x     
 * 
 * @param $batch The associative array of batch information
 * @param $content_type The type of content
 * @param $redirect (optional) Path to redirect to when the batch has finished processing
 */
function hook_cbr_core_calculate_similarity_batch_alter(&$batch, &$content_type, &$redirect) {
    if ($content_type == 'my_content_type') {
        $batch = array(
            'operations' => array(
                array('my_calculation', array($content_type)),
            ),
            'finished' => 'my_content_type_batch_finished',
            'title' => t('Processing CBR calculation'),
            'init_message' => t('CBR calculation is starting.'),
            'progress_message' => t('Processed @current out of @total.'),
            'error_message' => t('CBR core calculate has encountered an error.'),
            //Important: Path to file, where my_calculation is implemented
            'file' => drupal_get_path('module', 'my_module') . '/my_file.inc',
        );
        //Redirect to front page 
        $redirect = "/node";
    }
}

/**
 * This hook alters the batch context during calculation.
 * You may used it in a custom implemented field to add field information like content type 
 * and attribute weight to $context['sandbox']['fieldinfo'][$field_name]['type'] and 
 * $context['results']['fieldinfo'][$field_name]['weight']
 * 
 * This hook is called multiply time during the batch process. 
 * For more information see the documention for the batch api on drupal.org or the example at
 * https://www.drupal.org/node/180528
 * 
 * @param $context is an array that will contain information about the
 *   status of the batch. The values in $context will retain their
 *   values as the batch progresses. (from https://www.drupal.org/node/180528)
 * @param $content_type
 */
function hook_cbr_core_batch_calculation_context_alter(&$context, &$content_type) {
    //We change the attribute weight for the field type "my_type"
    //Get all field names and types
    $fields = field_info_instances('node', $content_type);
    foreach ($fields as $field_name => $value) {
        $field_info = field_info_field($field_name);
        $field_type = $field_info['type'];
        //Add the fieldinfo to $context if it is your type
        if ($field_type == 'my_type') {
            $context['sandbox']['fieldinfo'][$field_name]['type'] = $field_type;
            $context['results']['fieldinfo'][$field_name]['weight'] = 7;
        }
    }
}

/**
 * This hook alters $context before the normalization of the similarity
 * You may use it for overriding the normalization of the similarity
 * @param $context is an array that will contain information about the
 *   status of the batch. The values in $context will retain their
 *   values as the batch progresses. (from https://www.drupal.org/node/180528)
 */
function hook_cbr_core_batch_normalization_context_alter(&$context) {
    //This is an example implemention of the hook
    //We override the normalization. 
    //Caution, therefore, a larger similarity means less consensus. (Inverted from normal behavior)
    //You may have to update your view!
    foreach ($context['results']['fields'] as $field_name => $field_factors_per_nid_tuple) {
        foreach ($field_factors_per_nid_tuple as $nid_tuple => $field_factor) {
            $data_per_nid_tuble[$nid_tuple][$field_name] = $field_factor;
        }
    }

    //Skip the default implementation
    $context['results']['fields'] = array();

    //From cbr_core_calculate.inc:
    //We sum up the similarity calculated by each field to one similarity 
    //Under consideration of the weight of the field
    foreach ($data_per_nid_tuble as $nid_tuple => $field_array) {
        $sum = 0;
        $similarity = 0;
        foreach ($field_array as $field_name => $current_similarity) {
            $weight = $context['results']['fieldinfo'][$field_name]['weight'];
            $sum += $weight;
            $similarity += ($current_similarity * $weight);
        }
        $similarity = $sum > 0 ? $similarity / $sum : 0;
        $context['results']['similarity'][$nid_tuple] = $similarity;
    }
}

/**
 * This hook alters $context before saving the factors to database.
 * You may use it for additional calculations over all factors 
 * or alternativ storages. To turn off the default saving via cbr_core_calculate.inc, 
 * use $context['results']['similarity'] = array();  
 * 
 * @param $context is an array that will contain information about the
 *   status of the batch. The values in $context will retain their
 *   values as the batch progresses. (from https://www.drupal.org/node/180528) 
 */
function hook_cbr_core_batch_save_context_alter(&$context) {
    //This is an example implemention of the hook
    //Round all similarities to 2 decimals
    foreach ($context['results']['similarity'] as $nid_tuple => $similarity) {
        $context['results']['similarity'][$nid_tuple] = round($similarity, 2);
    }
}
