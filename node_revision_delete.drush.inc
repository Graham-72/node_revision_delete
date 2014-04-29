<?php

/**
 * @file
 * Drush integration for node_revision_delete
 */

/**
 * Implements COMMANDFILE_drush_command().
 */
function node_revision_delete_drush_command() {
  $items = array();

  $items['node-revision-delete'] = array(
    'description' => "Deletes old node revisions based on the currently saved settings at admin/config/content/node_revision_delete.",
    'options' => array(
      'loops' => 'The amount of times to call node_revision_delete_cron(). Defaults to 1.',
    ),
    'examples' => array(
      'drush nrd --loops=10' => 'Calls node_revision_delete_cron() 10 times.',
    ),
    'aliases' => array('nrd'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  return $items;
}

/**
 * Implements drush_COMMANDFILE_COMMANDNAME().
 */
function drush_node_revision_delete() {
  $loops = drush_get_option('loops', 1);
  if (!is_numeric($loops) || $loops < 1) {
    return drush_set_error('NODE_REVISION_DELETE_WRONG_LOOPS', dt('Loops option must be an integer greater than zero'));
  }
  while ($loops) {
    node_revision_delete_cron();
    $loops--;
  }
}
