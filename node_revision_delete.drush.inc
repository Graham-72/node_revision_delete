<?php

/**
 * @file
 * Drush commands related to the Node Revision Delete module.
 */

/**
 * Implements hook_drush_command().
 */
function node_revision_delete_drush_command() {
  $items['nrd-delete-cron-run'] = [
    'description' => dt('Configures how many revisions delete per cron run.'),
    'arguments' => [
      'quantity' => dt('Revisions quantity to delete per cron run.'),
    ],
    'aliases' => ['nrd-dcr'],
    'examples' => [
      'nrd-delete-cron-run' => dt('Show how many revisions the module will delete per cron run.'),
      'nrd-delete-cron-run 50' => dt('Configure the module to delete 50 revisions per cron run.'),
    ],
  ];
  return $items;
}

/**
 * Implements hook_drush_help().
 */
function node_revision_delete_drush_help($section) {
  switch ($section) {
    case 'meta:node_revision_delete:title':
      return dt('Node Revision Delete commands');

    case 'drush:nrd-delete-cron-run':
      return dt('Configures how many revisions delete per cron run.');
  }
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_node_revision_delete_nrd_delete_cron_run_validate() {
  $args = func_get_args();

  // Check for only one argument.
  if (count($args) > 1) {
    return drush_set_error('NODE_REVISION_DELETE_INVALID_ARGUMENT', dt('This command use only one argument as quantity.'));
  }

  // Check for integer quantity.
  if (isset($args[0]) && !ctype_digit($args[0])) {
    return drush_set_error('NODE_REVISION_DELETE_INVALID_ARGUMENT', dt('You must specify digits for the quantity'));
  }
}

/**
 * Callback for the nrd-delete-cron-run command.
 */
function drush_node_revision_delete_nrd_delete_cron_run() {
  $args = func_get_args();
  // Getting an editable config because we will get and set a value.
  $config = \Drupal::service('config.factory')->getEditable('node_revision_delete.settings');

  if (isset($args[0])) {
    // Saving the values in the config.
    $config->set('node_revision_delete_cron', $args[0]);
    $config->save();

    $message = dt('The module was configurated to delete @revisions revisions per cron run.', ['@revisions' => $args[0]]);
    drush_log($message, 'success');
  }
  else {
    // Getting the values from the config.
    $revisions = $config->get('node_revision_delete_cron');
    $message = dt('The quantity of revisions to delete per cron run is: @revisions.', ['@revisions' => $revisions]);
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
function _node_revision_delete_drush_plural($count, $singular, $plural, array $args = [], array $options = []) {
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
        return dt(strtr($plural, ['@count' => '@count[' . $index . ']']), $args, $options);
    }
  }
}
