<?php

/**
 * @file
 * Calls results to administration's pages for the Node Revision Delete module.
 */

/**
 * Page callback: Form constructor for Node Revision Delete administration form.
 */
function node_revision_delete_form($form, &$form_state) {
// Table header.
  $header = array(
    t('Content type'),
    t('Machine name'),
    t('Minimum to keep'),
    t('Minimum age'),
    t('When to delete'),
    t('Candidate nodes'),
    t('Operations'),
  );

// Table rows.
  $rows = array();

// Getting the config variables.
  $node_revision_delete_track = variable_get('node_revision_delete_track', array());
// Looking for all the content types.
  $content_types = node_type_get_types();
  $exists_candidates_nodes = FALSE;

  foreach ($content_types as $content_type) {

    if (FALSE) {

    }
    else {
      $minimum_revisions_to_keep = t('Untracked');
      $candidate_nodes = '-';
      $when_to_delete = t('Untracked');
      $minimum_age_to_delete = t('Untracked');
    }

//      'link' => l(t('edit'), 'admin/structure/types/manage/' . $type, array('query' => array('destination' => 'admin/config/content/node_revision_delete'))),
    $dropdown = 'Options';

    $rows[] = [
      $content_type->name,
      $content_type->type,
      $minimum_revisions_to_keep,
      $minimum_age_to_delete,
      $when_to_delete,
      $candidate_nodes,
      $dropdown,
    ];
  }

  $form['current_configuration'] = array(
    '#theme' => 'table',
    '#caption' => t('Current configuration'),
    '#header' => $header,
    '#rows' => $rows,
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'node_revision_delete') . '/css/node_revision_delete.css',
      ),
    ),
  );

  // Available options for node_revision_delete_cron variable.
  $options_node_revision_delete_cron = [
    10 => 10,
    20 => 20,
    50 => 50,
    100 => 100,
    200 => 200,
    500 => 500,
    1000 => 1000,
  ];

  $form['node_revision_delete_cron'] = array(
    '#type' => 'select',
    '#title' => t('How many revisions do you want to delete per cron run?'),
    '#description' => t('Deleting node revisions is a database intensive task. Increase this value if you think that the server can handle more deletions per cron run.'),
    '#options' => $options_node_revision_delete_cron,
    '#default_value' => variable_get('node_revision_delete_cron'),
  );

    // Available options for node_revision_delete_time variable.
    $options_node_revision_delete_time = [
      'never' => t('Never'),
      'every_time' => t('Every time cron runs'),
      'everyday' => t('Everyday'),
      'every_week' => t('Every Week'),
      'every_10_days' => t('Every 10 Days'),
      'every_15_days' => t('Every 15 Days'),
      'every_month' => t('Every Month'),
      'every_3_months' => t('Every 3 Months'),
      'every_6_months' => t('Every 6 Months'),
      'every_year' => t('Every Year'),
    ];

  $form['node_revision_delete_time'] = array(
    '#type' => 'select',
    '#options' => $options_node_revision_delete_time,
    '#title' => t('How often should revision be deleted while cron runs?'),
    '#default_value' => variable_get('node_revision_delete_time'),
  );
// Providing the option to run now the batch job.
  if ($exists_candidates_nodes) {
    $disabled = FALSE;
    $description = t('This will start a batch job to delete old revisions for tracked content types.');
  }
  else {
    $disabled = TRUE;
    $description = t('There not exists candidates nodes with revisions to delete.');
  }
  $form['run_now'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete revisions now'),
    '#description' => $description,
    '#disabled' => $disabled,
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Form submit handler for the settings form.
 */
function node_revision_delete_form_submit($form, &$form_state) {
  variable_set('node_revision_delete_cron', $form_state['values']['node_revision_delete_cron']);
  variable_set('node_revision_delete_time', $form_state['values']['node_revision_delete_time']);

  $node_revision_delete_content_types = node_revision_delete_content_types();
  if ($form_state['values']['run_now'] && count($node_revision_delete_content_types)) {
// Set up the batch job.
    $operations = array();
    foreach ($node_revision_delete_content_types as $content_type => $revisions_to_keep) {
      $operations[] = array('node_revision_delete_batch_process',
        array(
          $content_type,
          $revisions_to_keep,
        ),
      );
    }
    $batch = array(
      'operations' => $operations,
      'title' => t('Node Revision Delete batch job'),
      'init_message' => t('Starting...'),
      'error_message' => t('An error occurred'),
      'finished' => 'node_revision_delete_batch_finish',
      'file' => drupal_get_path('module', 'node_revision_delete') . '/node_revision_delete.batch.inc',
    );
    batch_set($batch);
  }
  drupal_set_message(t('Settings Saved Successfully.'));
}
