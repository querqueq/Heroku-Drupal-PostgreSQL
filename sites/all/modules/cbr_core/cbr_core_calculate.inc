<?php
/*
 *	Extensions made by j.Dorn
 *		- $min set because sometimes an error occured if $min is negative
 *		- more information given after finishing the batch process (required time in secs,
 *		  for how many fields the similarity is computed and for which fields
 *		- an additional argument $nid for the batch process to be able to restrict the
 *		  calculation on those related to only one node/case
 *		  assumption $min & $max have not changed by new node
 *		- under the assumption that similarity is defined as inverse relation
 *		  (i.e. similarity(nid1,nid2) = similarity(nid2,nid1), the inverse 
 *		  calculation is not required (at the moment inverse relations are not calculated
 *		- an additional phase for preprocessing where the fieldinfo data is stored in variables
 *		- similarity for entityreference is defined at the moment only for project_technology
 */
function microtime_float()
{
    list($usec, $sec) = explode(" ", microtime());
    return ((float)$usec + (float)$sec);
}

function cbr_core_calculate_similarity($content_type, $nid=0, $redirect = NULL) {
	$batch = array(
        'operations' => array(
            array('cbr_core_batch_preprocessing', array($content_type, $nid)),
            array('cbr_core_batch_calculation', array($content_type, $nid)),
            array('cbr_core_batch_save', array($content_type, $nid)),
        ),
        'finished' => 'cbr_core_batch_finished',
        'title' => t('Processing CBR calculation'),
        'init_message' => t('CBR calculation is starting.'),
        'progress_message' => t('Processing @current out of @total.'),
        'error_message' => t('CBR core calculate has encountered an error.'),
        'file' => drupal_get_path('module', 'cbr_core') . '/cbr_core_calculate.inc',
    );

    //Allow other modules to alter $batch & $redirect 
    drupal_alter('cbr_core_calculate_similarity_batch', $batch, $content_type, $redirect);

    batch_set($batch);
    if ($redirect != NULL) {
        batch_process($redirect);
    
    }
}
function cbr_core_batch_preprocessing($content_type, $nid, &$context) {
    //Load some informations on first batch call
	assert_options(ASSERT_ACTIVE, 1);
	assert_options(ASSERT_WARNING, 0);
	assert_options(ASSERT_QUIET_EVAL, 1);
	assert_options(ASSERT_CALLBACK, 'cbr_assert_handler');
	$context['cbr_build_similarity_structure'] = 
		variable_get('cbr_build_similarity_structure', 'true');
    if (!isset($context['sandbox']['progress'])) {
        $context['sandbox']['progress'] = 1;
        $context['sandbox']['current_node'] = 0;
        $context['results']['start'] = microtime_float();
        $context['results']['num_calculations'] = 0;

       		//Get all Node-IDs
    	$num_nodes = db_query("SELECT count(nid) FROM {node} WHERE type = :type ORDER BY nid", array(':type' => $content_type))->fetchCol();
       	if($nid)
       		$context['results']['num_nodes'] = 1;
    		else $context['results']['num_nodes'] = $num_nodes['0'];
        //Get all field names and types with CBR functionallity 
        $fields = field_info_instances('node', $content_type);
        $registered_fields = variable_get('cbr_core_registered_fields', array());
        foreach ($fields as $field_name => $value) {
            $field_info = field_info_field($field_name);
            $field_type = $field_info['type'];
            //Check if the field type is one of our supported ones and cbr is enabled.
            if (in_array($field_type, $registered_fields) && 
            		$field_info['cbr_settings']['enabled'] == 1) {
                $context['results']['fieldinfo'][$field_name]['type'] = $field_type;
                $context['results']['fieldinfo'][$field_name]['weight'] = 
                	(float) $field_info['cbr_settings']['weight'];
				$context['results']['fieldinfo'][$field_name]['cardinality'] = 
					$field_info['cardinality'];
                $context['results']['fieldinfo'][$field_name]['max'] = 
                	$field_info['cbr_settings']['max_distance_value'];
                $context['results']['fieldinfo'][$field_name]['foreign keys'] = 
                	$field_info['foreign keys'];
            }
        }
    }
    $context['finished'] = 1;
    /*
     * Still missing here is a calculation of maximal distances to make the relation
     * actual distance to maximal distance correct, at the moment the maximal distance of 
     * a field can be set in a dialog
     */
}
function cbr_core_batch_calculation($content_type, $nid, &$context) {
	$context['finished'] = 0;
    //Load some informations on first batch call
	if (!isset($context['sandbox']['progress'])) {
        $context['sandbox']['progress'] = 2;
        $context['sandbox']['current_node'] = 0;

       		//Get all Node-IDs
    	$context['sandbox']['nids'] = db_query("SELECT nid FROM {node} WHERE type = :type ORDER BY nid", array(':type' => $content_type))->fetchCol();
    	if($nid)
    		$context['sandbox']['max'] = 1;
        	else $context['sandbox']['max'] = count($context['sandbox']['nids']);
	 }

    //Allow other modules to alter $context
    drupal_alter('cbr_core_batch_calculation_context', $context, $content_type);
	//set_time_limit(120);
    // With each pass through the callback, we calculate the distance of one node to all others of it's type
    $cbr_inverse_similarity = variable_get('cbr_inverse_similarity', 'true');
    if($nid)
    	$context['sandbox']['current_node'] = $nid;
   		else $context['sandbox']['current_node'] = array_shift($context['sandbox']['nids']);

    if ($context['sandbox']['current_node'] != NULL) {
        $node1 = node_load($context['sandbox']['current_node']);
        $nodes = node_load_multiple($context['sandbox']['nids']);
        foreach ($nodes as $node2) {
        	if($node1->nid!=$node2->nid) {
        		$weight_sum = 0;
        		$similarity_sum = 0;
            	foreach ($context['results']['fieldinfo'] as $field_name => $fieldinfo) {
            		$weight = (isset($fieldinfo['weight']['value'])?
            			$fieldinfo['weight']['value']:$fieldinfo['weight']);
            		if($weight > 0) {
						$distance = cbr_core_calculate_distance_per_field($node1, $node2, $field_name, $fieldinfo);
						$similarity = round((1 - (min(1.0, $distance / $fieldinfo['max']))),3);
						assert('$similarity >= 0');
						assert('$similarity <= 1');
						$similarity_sum += $similarity*$weight;
						$weight_sum += $weight;
						if($distance > $fieldinfo['max']) {
							$context['results']['fieldinfo'][$field_name]['max'] = $distance;
						}
                	} // weight > 0
                } // all fields
                $weighted_simularity = round(($similarity_sum / $weight_sum),3);
                if($context['cbr_build_similarity_structure']) {
                	$context['results']['similarity'][$node1->nid . '|' . $node2->nid] = 
                		$weighted_simularity;
                }
                else {
                	cbr_core_save_similarity($node1->nid, $node2->nid, $weighted_simularity);
                	cbr_core_save_similarity($node2->nid, $node1->nid, $weighted_simularity);
                }
            	$context['results']['num_calculations']++;
            }
        }
    }
	
    //If nids is empty, we are done
    if (count($context['sandbox']['nids']) == 0) {
        $context['finished'] = 1;
    } else {
        // Update our progress information.
        $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
        $context['sandbox']['progress'] ++;
        $context['message'] = t('Now processing Node @nid', array('@nid' => $context['sandbox']['nids'][0]));
    }
}

//We calculate the similarity for a pair of nodes for a certain field
function cbr_core_calculate_distance_per_field($node1, $node2, $field_name, &$fieldinfo) {
    //Multiple modules can implement hook_calculate_cbr_distance_per_field. Loop over all of them.
    foreach (module_implements('calculate_cbr_distance_per_field') as $module) {
        // Calling all modules implementing hook_calculate_cbr_distance_per_field and 
        // Returning the result as factor if not NULL
        $result = module_invoke($module, 'calculate_cbr_distance_per_field', $node1, $node2, $field_name, $fieldinfo);
        if (!is_null($result)) {
            return $result;
        }
    }

    //Otherwise use the default implementation
    //Get the field's language
    $field_language1 = field_language('node', $node1, $field_name);
    $field_language2 = field_language('node', $node2, $field_name);

    //Different field types have different calculations
    switch ($fieldinfo['type']) {
        case 'cbr_number':
        case 'cbr_computed':
            $node1_fieldvalue = cbr_core_get_field_value($node1, $field_name, $field_language1, 'value');
            $node2_fieldvalue = cbr_core_get_field_value($node2, $field_name, $field_language2, 'value');
            if($fieldinfo['type']=='cbr_computed') {
            	if($node1_fieldvalue=='NULL' && $node2_fieldvalue=='NULL') {
            		$distance = 0;  // 1-0
            	}
            	else if($node1_fieldvalue=='NULL' || $node2_fieldvalue=='NULL') {
            		$distance = 1;
            	}
            	else {
            		$distance = abs($node1_fieldvalue - $node2_fieldvalue);
            	}
			}            	
            else $distance = abs($node1_fieldvalue - $node2_fieldvalue);
            break;
        case 'cbr_range':
            $node1_field_to = cbr_core_get_field_value($node1, $field_name, $field_language1, 'to');
            $node1_field_from = cbr_core_get_field_value($node1, $field_name, $field_language1, 'from');
            $node2_field_to = cbr_core_get_field_value($node2, $field_name, $field_language2, 'to');
            $node2_field_from = cbr_core_get_field_value($node2, $field_name, $field_language2, 'from');
            $distance = abs($node1_field_from - $node2_field_from) + abs($node1_field_to - $node2_field_to);
            break;
        case 'cbr_taxonomy':
            $node1_tid = cbr_core_get_field_value($node1, $field_name, $field_language1, 'tid');
            $node2_tid = cbr_core_get_field_value($node2, $field_name, $field_language2, 'tid');
            if ($node1_tid == $node2_tid) {
                $distance = 0; //is inverted later to 1
            } else {
                $parents1 = taxonomy_get_parents_all($node1_tid);
                $parents2 = taxonomy_get_parents_all($node2_tid);
                $distance = cbr_core_calculate_taxonomy_tree_distance($parents1, $parents2);
            }
            break;
        case 'entityreference':
        	/*
        	 * EntityReference is realized here only for the field field_project_technology where unlimited taxonomy terms are compared
        	 * if other comparisons are made nothing is calculated at the moment
        	 */
            $node1_tids = cbr_core_get_field_values($node1, $field_name, $field_language1, 'tid');
            $node2_tids = cbr_core_get_field_values($node2, $field_name, $field_language2, 'tid');
            $distance=0;
            if(isset($fieldinfo['foreign keys']))
            {
                if(isset($fieldinfo['foreign keys']['taxonomy_term_data'])) {
				//if($field_name=="field_project_technology") {
					$num_relations = count($node1_tids) * count($node2_tids);
					$text='';
					if($num_relations) {
						$distances = array();
						foreach($node1_tids as $tid1) {
							foreach($node2_tids as $tid2) {
								if($tid1!=$tid2) {
									$parents1 = taxonomy_get_parents_all($tid1);
									$parents2 = taxonomy_get_parents_all($tid2);
									$distance = cbr_core_calculate_taxonomy_tree_distance($parents1, $parents2);
									$distances[] = $distance;
								}
							}
						}
						sort($distances);
						$distance = getSumOfNearest($distances, 3);
					}
					else {
						$distance = $fieldinfo['max'];
					}            	
				}
            }
        	break;
    }
    return $distance;
}
function getSumOfNearest($distances, $num) {
	if($num==0) return 0; 
	if(empty($distances))
		return $num*8;   // should be a variable that can be configured 8 as largest 
	$dist = array_shift($distances);
	return $dist+getSumOfNearest($distances, --$num);
}
	
function cbr_core_calculate_taxonomy_tree_distance($parents1, $parents2) {
    if (empty($parents1)) {
        return count($parents2);
    }
    if (empty($parents2)) {
        return count($parents1);
    }
	//return  count($parents1) + count($parents2);
    //Select the last element of the parents array
    //This is the root of the tree
    $term1 = end($parents1);
    $term2 = end($parents2);

    //While both have the same term id, we move down the tree
    while ($term1->tid == $term2->tid) {
        array_pop($parents1);
        array_pop($parents2);
        $term1 = end($parents1);
        $term2 = end($parents2);
    }
    //We found the Lowest Common Ancestor (LCA)! 
    //$parents1 and $parents2 contain the path to the LCA, so we return the sum 
    //of the number of the elements, because it's the distance between the two elements
    return count($parents1) + count($parents2);
}

function cbr_core_batch_save($content_type, $nid, &$context) {
	if (!isset($context['sandbox']['progress']))
        $context['sandbox']['progress'] = 3;
    //Allow other modules to alter $context
    drupal_alter('cbr_core_batch_save_context', $context);
    if($context['cbr_build_similarity_structure']) {
		foreach ($context['results']['similarity'] as $nid_tuple => $similarity) {
			$nid_tuple_array = explode('|', $nid_tuple);
			cbr_core_save_similarity($nid_tuple_array[0], $nid_tuple_array[1], $similarity);
			cbr_core_save_similarity($nid_tuple_array[1], $nid_tuple_array[0], $similarity);
		}
    }
    foreach ($results['fieldinfo'] as $field_name => $field_info) {
        //Check if the field type is one of our supported ones and cbr is enabled.
		if($field_name=="field_project_technology") {
			$updated_field['field_name'] = $field_name;
			$updated_field['cbr_settings']['enabled'] = true;
			$updated_field['cbr_settings']['weight'] = $field_info['weight'];
			$updated_field['cbr_settings']['max_distance_value'] = $field_info['max'];
			field_update_field($updated_field);
		}
    }
    
    $context['results']['time_end'] = microtime_float();
	$context['results']['time'] = $context['results']['time_end'] - 			
		$context['results']['start'];
}

/**
 * Batch 'finished' callback
 */
function cbr_core_batch_finished($success, $results, $operations) {
    if ($success) {
    	if($results['num_calculations'])
    	{
    		if(variable_get('cbr_detailed_results'))
    		{
				$message1 = 'Successful calculated '.$results['num_calculations'].' times a similarity in '.round($results['time'],2).' seconds for ';
				$num_fields = 0;
				$message3 = '';
				foreach ($results['fieldinfo'] as $field_name => $field_info)
					if((isset($field_info['weight']['value']) && $field_info['weight']['value'] > 0) ||
						$field_info['weight']>0) {
						$num_fields++;
						$message3 .= '<br>Field of type '.$field_info['type'].
							' with name '.$field_name.
							' cardinality '.$field_info['cardinality'].
							' and weight '.((isset($field_info['weight']['value']))?$field_info['weight']['value']:$field_info['weight']);
						}
				if($results['num_nodes']==1)
					$message2 = $num_fields.' fields for one case! ';
					else $message2 = $num_fields.' fields for '.$results['num_nodes'].' cases! ';

				drupal_set_message(t($message1.$message2.$message3));
			}
			else if($results['num_nodes']==1)
				drupal_set_message(t('Successful calculated  similarity for one case in '.round($results['time'],2).' seconds ! '));
				else drupal_set_message(t('Successful calculated  similarity in '.round($results['time'],2).' seconds for '.$results['num_nodes'].' cases! '));
        }
        else drupal_set_message("No calculation took place");
    } else {
        // An error occurred.
        // $operations contains the operations that remained unprocessed.
        $error_operation = reset($operations);
        $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
        drupal_set_message($message, 'error');
    }
    drupal_goto('/cbr/cases');
}

function cbr_core_get_field_value($node, $field_name, $field_language, $field_column) {
    //Transform object into array
    $node_values = get_object_vars($node);

    if (array_key_exists($field_language, $node_values[$field_name]) && array_key_exists(0, $node_values[$field_name][$field_language])) {
        return $node_values[$field_name][$field_language][0][$field_column];
    }
    return 0;
}
function cbr_core_get_field_values($node, $field_name, $field_language, $field_column) {
    //Transform object into array
    $values = array();
    $node_values = get_object_vars($node);
    $i=0;
	while(array_key_exists($field_language, $node_values[$field_name]) && array_key_exists($i, $node_values[$field_name][$field_language]))
	{
		$values[] = $node_values[$field_name][$field_language][$i++]['target_id'];
			}
    return $values;
}


function cbr_core_save_similarity($nid1, $nid2, $similarity) {
    $result = db_select('cbr_similarity_factor', 'c')
                    ->fields('c')
                    ->condition('nid1', $nid1)
                    ->condition('nid2', $nid2)
                    ->execute()->fetchAll();
    if (count($result) != 0) {
        db_update('cbr_similarity_factor')->fields(array('similarity_factor' => $similarity))
                ->condition('nid1', $nid1)
                ->condition('nid2', $nid2)
                ->execute();
    } else {
        db_insert('cbr_similarity_factor')->fields(array(
                    'nid1' => $nid1,
                    'nid2' => $nid2,
                    'similarity_factor' => $similarity,
                ))
                ->execute();
    }
}
