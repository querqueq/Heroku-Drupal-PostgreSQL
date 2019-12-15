<?php

/**
* Implements hook_field_views_data().
*
* Views integration to provide reverse relationships on entityreference fields.
*/
function role_field_field_views_data($field) {
  $data = field_views_field_default_views_data($field);

  foreach ($data as $table_name => $table_data) {
    $field_name = $field['field_name'] . '_value';
    $data[$table_name][$field_name]['filter'] = array(
      'handler' => 'role_field_handler_filter_all_user_roles',
      'numeric' => TRUE,
      'allow empty' => TRUE,
    );
  }
  return $data;
}

/**
 * Class role_field_handler_filter_all_user_roles
 */
class role_field_handler_filter_all_user_roles extends views_handler_filter_user_roles {
  // Override the options to also allow filtering on 'anonymous' or 'authenticated users'
  function get_value_options() {
    $this->value_options = user_roles();
  }
}
