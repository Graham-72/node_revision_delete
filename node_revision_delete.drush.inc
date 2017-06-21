<?php

/**
 * @file
 * Drush commands related to the Node Revision Delete module.
 */

/**
 * Implements hook_drush_command().
 */
function node_revision_delete_drush_command() {
  $items['node-revision-delete'] = array(
    'description' => dt('Deletes old node revisions for a given content type.'),
    'arguments' => array(
      'type' => dt("A content type's machine name."),
      'revisions' => dt('The maximum amount of revisions to keep per node for this content type.'),
    ),
    'required-arguments' => TRUE,
    'examples' => array(
      'drush nrd article 50' => dt('Keeps the latest 50 revisions of every article. Deletes the rest.'),
    ),
    'aliases' => array('nrd'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );
  $items['nrd-delete-cron-run'] = array(
    'description' => dt('Configures how many revisions delete per cron run.'),
    'arguments' => array(
      'quantity' => dt('Revisions quantity to delete per cron run.'),
    ),
    'aliases' => array('nrd-dcr'),
    'examples' => array(
      'nrd-delete-cron-run' => dt('Show how many revisions the module will delete per cron run.'),
      'nrd-delete-cron-run 50' => dt('Configure the module to delete 50 revisions per cron run.'),
    ),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function node_revision_delete_drush_help($section) {
  switch ($section) {
    case 'meta:node_revision_delete:title':
      return dt('Node Revision Delete commands');

    case 'drush:node-revision-delete':
      return dt('Deletes old node revisions for a given content type.');

    case 'drush:nrd-delete-cron-run':
      return dt('Configures how many revisions delete per cron run.');
  }
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_node_revision_delete_validate($content_type, $revisions_to_keep) {
  // Make sure the content type exists.
  $content_types = array_keys(node_type_get_types());
  if (!in_array($content_type, $content_types)) {
    drush_set_error('NODE_REVISION_DELETE_WRONG_TYPE', dt('The content type "@type" does not exist. Available content types are @types', array(
      '@type' => $content_type,
      '@types' => implode(', ', $content_types),
    )));
  }

  // Make sure the number of revisions is a positive integer.
  // Based on Drupal's element_validate_integer_positive().
  if (!is_numeric($revisions_to_keep) || intval($revisions_to_keep) != $revisions_to_keep || $revisions_to_keep <= 0) {
    drush_set_error('NODE_REVISION_DELETE_WRONG_REVISIONS', dt('The amount of revisions to keep must be a positive integer.'));
  }
}

/**
 * Implements drush_COMMANDFILE_COMMANDNAME().
 */
function drush_node_revision_delete($content_type, $revisions_to_keep) {
  // Set up the batch job.
  $batch = array(
    'operations' => array(
      array('node_revision_delete_batch_process',
        array($content_type,
          $revisions_to_keep,
        ),
      ),
    ),
    'title' => dt('Node Revision Delete batch job'),
    'init_message' => dt('Starting...'),
    'error_message' => dt('An error occurred'),
    'finished' => 'node_revision_delete_batch_finish',
    'file' => drupal_get_path('module', 'node_revision_delete') . '/node_revision_delete.batch.inc',
  );

  // Start the batch job.
  batch_set($batch);
  drush_backend_batch_process();
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_node_revision_delete_nrd_delete_cron_run_validate() {
  $args = func_get_args();

  // Check for only one argument.
  if (count($args) > 1) {
    drush_set_error('DRUSH_NODE_REVISION_DELETE_ERROR', dt('This command use only one argument as quantity.'));
    return;
  }

  // Check for integer quantity.
  if (isset($args[0]) && !ctype_digit($args[0])) {
    drush_set_error('DRUSH_NODE_REVISION_DELETE_ERROR', dt('You must specify digits for the quantity'));
    return;
  }
}

/**
 * Callback for the nrd-delete-cron-run command.
 */
function drush_node_revision_delete_nrd_delete_cron_run() {
  $args = func_get_args();

  if (isset($args[0])) {
    // Saving the values in the config.
    variable_set('node_revision_delete_cron', $args[0]);

    $message = dt('The module was configurated to delete @revisions revisions per cron run.', array('@revisions' => $args[0]));
    drush_log($message, 'success');
  }
  else {
    // Getting the values from the config.
    $revisions = variable_get('node_revision_delete_cron');
    $message = dt('The quantity of revisions to delete per cron run is: @revisions.', array('@revisions' => $revisions));
    drush_print($message);
  }
}

/**
 * Formats a plural string containing a count of items.
 *
 * This function ensures that the string is pluralized correctly. Since dt() is
 * called by this function, make sure not to pass already-localized strings to
 * it.
 *
 * For example:
 * @code
 *   $output = _node_revision_delete_drush_plural($node->comment_count, '1 comment', '@count comments');
 * @endcode
 *
 * @param int $count
 *   The item count to display.
 * @param string $singular
 *   The string for the singular case. Make sure it is clear this is singular,
 *   to ease translation (e.g. use "1 new comment" instead of "1 new"). Do not
 *   use @count in the singular string.
 * @param string $plural
 *   The string for the plural case. Make sure it is clear this is plural, to
 *   ease translation. Use @count in place of the item count, as in
 *   "@count new comments".
 * @param array $args
 *   An associative array of replacements to make after translation. Instances
 *   of any key in this array are replaced with the corresponding value.
 *   Based on the first character of the key, the value is escaped and/or
 *   themed. See format_string(). Note that you do not need to include @count
 *   in this array; this replacement is done automatically for the plural case.
 * @param array $options
 *   An associative array of additional options. See dt() for allowed keys.
 *
 * @return string
 *   A translated string.
 *
 * @see dt()
 */
function _node_revision_delete_drush_plural($count, $singular, $plural, array $args = array(), array $options = array()) {
  $args['@count'] = $count;
  if ($count == 1) {
    return dt($singular, $args, $options);
  }

  // Get the plural index through the gettext formula.
  $index = (function_exists('locale_get_plural')) ? locale_get_plural($count, isset($options['langcode']) ? $options['langcode'] : NULL) : -1;
  // If the index cannot be computed, use the plural as a fallback (which
  // allows for most flexiblity with the replaceable @count value).
  if ($index < 0) {
    return dt($plural, $args, $options);
  }
  else {
    switch ($index) {
      case "0":
        return dt($singular, $args, $options);

      case "1":
        return dt($plural, $args, $options);

      default:
        unset($args['@count']);
        $args['@count[' . $index . ']'] = $count;
        return dt(strtr($plural, array('@count' => '@count[' . $index . ']')), $args, $options);
    }
  }
}
