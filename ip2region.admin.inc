<?php
// $Id: ip2region.admin.inc,v1 2009/05/25 11:34:17 Paul Flewelling Exp $

/**
 * @file
 * Admin functions for ip2region module.
 */


/**
 * Menu callback and form builder for admin/settings/ip2region.
 */
function ip2region_admin_list() {  
	$output = '';

	$result = db_query('SELECT uid, library_name, {term_data}.name, ip_from, ip_to FROM {ip2region},{vocabulary} INNER JOIN {term_data} ON {term_data}.vid={vocabulary}.vid WHERE {vocabulary}.name = "Regions" AND {term_data}.tid = {ip2region}.region');
	$rows = array();
	while($field = db_fetch_object($result)) {
		$rows[] = array(check_plain($field->library_name), check_plain($field->name), check_plain($field->ip_from), check_plain($field->ip_to), l(t('delete'), "admin/settings/ip2region/delete/$field->uid"));
	}
	
	if(count($rows) == 0) {
		$rows[] = array(array('data' => t('No region/ip records defined.'), 'colspan' => '4'));
	}
	
	$header = array(t('Library Name'), array('data' => t('Region Name')), array('data' => t('IP From')), array('data' => t('IP To')));
	$output .= theme('table', $header, $rows);
	$output .= '<h2>'.l(t('Add new region/ip range'),'admin/settings/ip2region/add') .'</h2>';	
	return $output;
}


function ip2region_admin_add() {
  $form = array();
  $form['ip2region'] = array(
    '#type' => 'fieldset',
    '#title' => t('Region'),
    '#description' => t('Select a region name & enter a single IP Address or a optionally an IP Address Range')
  );  
  $form['ip2region']['library_name'] = array(
    '#title' => t('Library Name'),
    '#type' => 'textfield',
    '#description' => t('The Library Name'),
    '#size' => 30,
    '#maxlength' => 30,
  );
  $form['ip2region']['region_name'] = array(
    '#title' => t('Region Name'),
    '#type' => 'select',
    '#default_value' => is_numeric($field['parent']) ? $field['parent'] : 0,
    '#options' => _content_taxonomy_get_all_terms(),
    '#description' => t(''),
  );
  $form['ip2region']['ip_from'] = array(
    '#type' => 'textfield', 
    '#title' => t('IP address'),
    '#description' => t('The base IP address, i.e. 192.168.0.1'),
    '#size' => 15,
    '#maxlength' => 15,
  );
  $form['ip2region']['ip_to'] = array(
    '#type' => 'textfield', 
    '#title' => t('IP address'),
    '#description' => t('The last IP address (required if this is a range), i.e. 192.168.0.1'),
    '#size' => 15,
    '#maxlength' => 15,
  );
/** Not using CIDR for now, will refactor if needed.
  $form['ip2region']['cidr'] = array(
    '#type' => 'textfield', 
    '#title' => t('CIDR Code'),
    '#description' => t('The CIDR indicating the range for the IP, see the following link http://en.wikipedia.org/wiki/Classless_Inter-Domain_Routing'),
    '#size' => 2,
    '#maxlength' => 2,
  );
**/
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Create Region/IP'),
  );
  return $form;
}


function ip2region_admin_add_submit($form, &$form_state) {
  $result = db_query("INSERT INTO {ip2region} (library_name, region, ip_from, ip_to) VALUES('%s', '%d', '%s', '%s')", $form_state['values']['library_name'], $form_state['values']['region_name'], $form_state['values']['ip_from'], $form_state['values']['ip_to']);
	if ($result) drupal_set_message(t('The Region/IP has been created.'));
}
/**
 * Validation callback.
 
function _ip2region_admin_add_validate($element, $form_state) {
     // Check for duplicate key values to prevent unexpected data loss.
      if (!empty($element['#value'])) {
        $lines = explode("\n", $element['#value']);
        $existing_keys = array();
        $invalid_maps = array();
        $group = '';

        foreach ($lines as $line) {
          $matches = array();
          $line = trim($line);
          if (preg_match('/[0-9a-fA-F]\,(([0-1]?[0-9]{1,2}\.)|(2[0-4][0-9]\.)|(25[0-5]\.)){3}(([0-1]?[0-9]{1,2})|(2[0-4][0-9])|(25[0-5]))\,[0-9]{2}', $line, $matches)) {
            $key = $matches[1];
          }
          else {
            $invalid_maps[$line] = $line;
          }

          $existing_keys[$group][$key] = $key;
        }

        if (!empty($invalid_maps)) {
          form_error($element, t('Each IP 2 Region map must be in the format \'Region,IP,CIDR\' (<em>Region Name</em>, <em>IP Address Range</em>, <em>CIDR</em>[optional]) eg.') .'<br/>'. t('\'Amberley, 172.23.0.0, 24\' (<em>Amberley Region</em>, <em>IP Address + CIDR = IP Address Range 172.23.0.0 - 172.23.0.255</em>)') .'<br/>'. t('\'Rangiora,172.23.8.133\' (<em>Rangiora</em>, <em>IP Address with no CIDR = single IP address</em>)') .'<br/>'. t('The following mappings are invalid:') . theme('item_list', $invalid_maps));
        }
      }
      return TRUE;
    }
 */



/**
* Menu callback; deletes a region/ip combination
*/
function ip2region_admin_delete($uid) {
	$id = $uid;
  $record = db_fetch_object(db_query("SELECT uid, name, ip_from, ip_to FROM {ip2region}, {term_data} WHERE uid='%d' AND tid=region", $uid));
  if (!$record) {
    drupal_not_found();
    return;
  }

  db_query('DELETE FROM {ip2region} WHERE uid = %d', $uid);
  cache_clear_all();
  drupal_set_message(t('The ip2region record for %name, %ip has been deleted.', array('%name' => $record->name, '%ip' => $record->ip)));
  drupal_goto('admin/settings/ip2region');
}


function ip2region_admin_test() {
  $form = array();
  $form['ip2region'] = array(
    '#type' => 'fieldset',
    '#title' => t('Region'),
    '#description' => t("Enter a 'test' User IP address to determine if ranges are working (your current IP address is %user_ip)", array('%user_ip' => ip2region_ip_address())),
  );  
  $form['ip2region']['test_ip'] = array(
    '#type' => 'textfield', 
    '#title' => t('TestIP address'),
    '#description' => t("A test IP address to simulate the User's IP"),
    '#size' => 15,
    '#maxlength' => 15,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Test IP'),
  );
  return $form;
}

function ip2region_admin_test_submit($form, &$form_state) {
	$users_ip = $form_state['values']['test_ip'];
	$region = ip2region_get_region_from_ip($users_ip);
	$region = $region['region_name'];
	drupal_set_message(t('The ip2region record for %region has matched.', array('%region' => $region)));
}

/** 
 * Helper function that returns the vocabulary for "Regions"
 *
 */
function _content_taxonomy_get_all_terms() {
  static $options = array();
  if (!count($options)) {
	  
    foreach (taxonomy_get_vocabularies() as $vocabulary) {
    	if ($vocabulary->name=="Regions") {
	      foreach (taxonomy_get_tree($vocabulary->vid) as $term) {
	        $options[$vocabulary->name][$term->tid] = str_repeat('- ', $term->depth) . $term->name;
	      }
	      return $options;
    	}
    } 
  }
  return;
}

