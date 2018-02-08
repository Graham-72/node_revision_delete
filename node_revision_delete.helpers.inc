<?php

/**
 * @file
 * Helper functions.
 */

/**
 * Save the content type config variable.
 *
 * @param string $content_type
 *   Content type machine name.
 * @param int $minimum_revisions_to_keep
 *   Minimum number of revisions to keep.
 * @param int $minimum_age_to_delete
 *   Minimum age in months of revision to delete.
 * @param int $when_to_delete
 *   Number of inactivity months to wait for delete a revision.
 */
function _node_revision_delete_save_content_type_config($content_type, $minimum_revisions_to_keep, $minimum_age_to_delete, $when_to_delete) {
  // Getting the variables with the content types configuration.
  $node_revision_delete_track = variable_get('node_revision_delete_track');
  // Creating the content type info.
  $content_type_info = array(
    'minimum_revisions_to_keep' => $minimum_revisions_to_keep,
    'minimum_age_to_delete' => $minimum_age_to_delete,
    'when_to_delete' => $when_to_delete,
  );
  // Adding the info into te array.
  $node_revision_delete_track[$content_type] = $content_type_info;
  // Saving the values in the config.
  variable_set('node_revision_delete_track', $node_revision_delete_track);
}

/**
 * Delete the content type config variable.
 *
 * @param string $content_type
 *   Content type machine name.
 *
 * @return bool
 *   Return TRUE if the content type config was deleted or FALSE if not exists.
 */
function _node_revision_delete_delete_content_type_config($content_type) {
  // Getting the variables with the content types configuration.
  $node_revision_delete_track = variable_get('node_revision_delete_track');
  // Checking if the config exists.
  if (isset($node_revision_delete_track[$content_type])) {
    // Deleting the value from the array.
    unset($node_revision_delete_track[$content_type]);
    // Saving the values in the config.
    variable_set('node_revision_delete_track', $node_revision_delete_track);
    return TRUE;
  }
  return FALSE;
}

/**
 * Return the list of candidate nodes for node revision delete.
 *
 * @param string $content_type
 *   Content type machine name.
 * @param int $minimum_revisions_to_keep
 *   Minimum number of revisions to keep.
 * @param int $minimum_age_to_delete
 *   Minimum age of revisions to be deleted.
 * @param int $when_to_delete
 *   Minimum inactivity age to wait for delete a revision.
 *
 * @return array
 *   Array of nids.
 */
function _node_revision_delete_candidates($content_type, $minimum_revisions_to_keep, $minimum_age_to_delete, $when_to_delete) {
  static $minimum_age_to_delete_time, $when_to_delete_time;

  // Get the time value for minimum age to delete.
  if (empty($minimum_age_to_delete_time)) {
    $minimum_age_to_delete_time = variable_get('node_revision_delete_minimum_age_to_delete_time')['time'];
  }

  // Get the time value for when to delete.
  if (empty($when_to_delete_time)) {
    $when_to_delete_time = variable_get('node_revision_delete_when_to_delete_time')['time'];
  }

  $params = array(
    ':content_type' => $content_type,
    ':max_revisions' => $minimum_revisions_to_keep,
    ':age' => strtotime($minimum_age_to_delete . ' ' . $minimum_age_to_delete_time . ' ago'),
    ':when' => strtotime($when_to_delete . ' ' . $when_to_delete_time . ' ago'),
  );

  $result = db_query('SELECT r.nid, count(*) as total
                     FROM {node} n
                     INNER JOIN {node_revision} r ON r.nid = n.nid
                     WHERE n.type = :content_type
                     AND r.timestamp < :age
                     AND n.changed < :when
                     GROUP BY r.nid
                     HAVING count(*) > :max_revisions
                     ORDER BY total DESC', $params);
  return $result->fetchCol();
}

/**
 * Return the time string for the config_name parameter.
 *
 * @param string $config_name
 *   The config name.
 * @param int $number
 *   The number for the $config_name parameter configuration.
 *
 * @return string
 *   The time string for the $config_name parameter.
 */
function _node_revision_delete_time_string($config_name, $number) {
  $config_name_time = variable_get('node_revision_delete_' . $config_name . '_time');
  // Is singular or plural?
  $time = _node_revision_delete_time_number_string($number, $config_name_time['time']);
  // Return the time string for the $config_name parameter.
  switch ($config_name) {
    case 'minimum_age_to_delete':
      return t('@number @time', array('@number' => $number, '@time' => $time));

    case 'when_to_delete':
      return t('After @number @time of inactivity', array('@number' => $number, '@time' => $time));
  }
}

/**
 * Update the max_number for a config name.
 *
 * We need to update the max_number in the existing content type configuration
 * if the new value (max_number) is lower than the actual, in this case the new
 * value will be the value for the content type.
 *
 * @param string $config_name
 *   Config name to update.
 * @param int $max_number
 *   The maximum number for $config_name parameter.
 */
function _node_revision_delete_update_time_max_number_config($config_name, $max_number) {
  $node_revision_delete_track = variable_get('node_revision_delete_track', array());
  $changed = FALSE;
  // Checking the when_to_delete value for all the configured content types.
  foreach ($node_revision_delete_track as $content_type => $content_type_info) {
    // If the new defined max_number is smaller than the defined when_to_delete
    // value in the config, we need to change the stored config value.
    if ($max_number < $content_type_info[$config_name]) {
      $node_revision_delete_track[$content_type][$config_name] = $max_number;
      $changed = TRUE;
    }
  }
  // Saving only if we have changes.
  if ($changed) {
    // Saving the values in the config.
    variable_set('node_revision_delete_track', $node_revision_delete_track);
  }
}

/**
 * Return the available values for time frequency.
 *
 * @param string $index
 *   The index to retrieve.
 *
 * @return string
 *   The index value (human readable value).
 */
function _node_revision_delete_time_value($index = NULL) {
  $options_node_revision_delete_time = array(
    'never' => t('Never'),
    'every_time' => t('Every time cron runs'),
    'every_hour' => t('Every Hour'),
    'everyday' => t('Everyday'),
    'every_week' => t('Every Week'),
    'every_10_days' => t('Every 10 Days'),
    'every_15_days' => t('Every 15 Days'),
    'every_month' => t('Every Month'),
    'every_3_months' => t('Every 3 Months'),
    'every_6_months' => t('Every 6 Months'),
    'every_year' => t('Every Year'),
    'every_2_years' => t('Every 2 Years'),
  );

  if (isset($index) && isset($options_node_revision_delete_time[$index])) {
    return $options_node_revision_delete_time[$index];
  }
  else {
    return $options_node_revision_delete_time;
  }
}

/**
 * Return the time option in singular or plural.
 *
 * @param string $number
 *   The number.
 * @param string $time
 *   The time option (days, weeks or months).
 *
 * @return string
 *   The singular or plural value for the time.
 */
function _node_revision_delete_time_number_string($number, $time) {
  // Time options.
  $time_options = array(
    'days' => array(
      'singular' => t('day'),
      'plural' => t('days'),
    ),
    'weeks' => array(
      'singular' => t('week'),
      'plural' => t('weeks'),
    ),
    'months' => array(
      'singular' => t('month'),
      'plural' => t('months'),
    ),
  );

  return $number == 1 ? $time_options[$time]['singular'] : $time_options[$time]['plural'];
}

/**
 * Returns if a number is a positive integer.
 *
 * @param int $number
 *   The time option (days, weeks or months).
 *
 * @return bool
 *   TRUE if the number is integer positive, FALSE otherwise.
 *
 * @see element_validate_integer_positive()
 */
function _node_revision_delete_is_integer_positive($number) {
  if ($number !== '' && (!is_numeric($number) || intval($number) != $number || $number <= 0)) {
    return FALSE;
  }
  return TRUE;
}

/**
 * Private function to perform revision deletion.
 *
 * @param int $nid
 *   The node whose oldest revisions will be deleted.
 * @param int $minimum_revisions_to_keep
 *   Max amount of revisions to keep for this node.
 * @param bool $dry_run
 *   TRUE to test run without deleting revisions but seeing the output results,
 *   FALSE for a real node revision delete.
 *
 * @return object
 *   stdClass with list containing an array of deleted revisions
 *   and pending containing a boolean where TRUE means that there
 *   are more revisions to delete for this node.
 */
function _node_revision_delete_do_delete($nid, $minimum_revisions_to_keep, $dry_run = FALSE) {
  // Getting the number of revisions to remove in each cron run.
  $max = variable_get('node_revision_delete_cron', 50);

  $node = new stdClass();
  $node->nid = $nid;
  $revisions = node_revision_list($node);
  // Keep recent revisions.
  $revisions = array_slice($revisions, $minimum_revisions_to_keep);
  // Reverse the list so we start deleting oldest revisions first.
  $revisions = array_reverse($revisions);
  // POPO to keep track of deleted revisions and whether there are more
  // to be deleted on a next run.
  $deleted_revisions = new stdClass();
  $deleted_revisions->count = 0;
  $deleted_revisions->pending = FALSE;
  foreach ($revisions as $revision) {
    $revision_id = $revision->vid;
    if ($dry_run ? $dry_run : node_revision_delete($revision_id)) {
      $deleted_revisions->count++;
      // Stop deleting if we hit the limit per cron run.
      if ($deleted_revisions->count == $max) {
        $deleted_revisions->pending = TRUE;
        break;
      }
    }
  }

  return $deleted_revisions;
}
