<?php

namespace Drupal\node_revision_delete\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Render\RendererInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\Core\Url;
use Drupal\Core\Entity\EntityManagerInterface;

/**
 * Class NodeRevisionDeleteAdminSettings.
 *
 * @package Drupal\node_revision_delete\Form
 */
class NodeRevisionDeleteAdminSettings extends ConfigFormBase {

  /**
   * Drupal\Core\Render\RendererInterface definition.
   *
   * @var Drupal\Core\Render\RendererInterface
   */
  protected $renderer;

  /**
   * Drupal\Core\Entity\EntityManagerInterface definition.
   *
   * @var Drupal\Core\Entity\EntityManagerInterface
   */
  protected $entityManager;

  /**
   * {@inheritdoc}
   */
  public function __construct(RendererInterface $renderer, EntityManagerInterface $entity_manager) {
    $this->renderer = $renderer;
    $this->entityManager = $entity_manager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('renderer'), $container->get('entity.manager')
    );
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [
      'node_revision_delete.settings',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'node_revision_delete_admin_settings';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    // Getting the config variables.
    $config = $this->config('node_revision_delete.settings');
    // Table header.
    $header = [
      $this->t('Content type'),
      $this->t('Machine name'),
      $this->t('Minimum to keep'),
      $this->t('Minimum age'),
      $this->t('When to delete'),
      $this->t('Candidate nodes'),
      $this->t('Operations'),
    ];
    // Table rows.
    $rows = [];
    // Getting the config variables.
    $node_revision_delete_track = $config->get('node_revision_delete_track');
    // Looking for all the content types.
    $content_types = $this->entityManager->getStorage('node_type')->loadMultiple();
    // Check if exists candidates nodes.
    $exists_candidates_nodes = FALSE;

    // Return to the same page after save the content type.
    $destination = Url::fromRoute('node_revision_delete.admin_settings')->toString();
    $destination_options = [
      'query' => ['destination' => $destination],
      'fragment' => 'edit-workflow',
    ];

    foreach ($content_types as $content_type) {
      $route_parameters = ['node_type' => $content_type->id()];
      // Operations dropbutton.
      $dropbutton = [
        '#type' => 'dropbutton',
        '#links' => [
          // Action to edit the content type.
          'edit' => [
            'title' => $this->t('Edit'),
            'url' => Url::fromRoute('entity.node_type.edit_form', $route_parameters, $destination_options),
          ],
        ],
      ];

      // Searching the revisions to keep for each content type.
      if (isset($node_revision_delete_track[$content_type->id()])) {
        // Minimum revisions to keep in the database.
        $minimum_revisions_to_keep = $node_revision_delete_track[$content_type->id()]['minimum_revisions_to_keep'];

        // Minimum age to delete (is a number, 0 for none).
        $minimum_age_to_delete_number = $node_revision_delete_track[$content_type->id()]['minimum_age_to_delete'];
        $minimum_age_to_delete = (bool) $minimum_age_to_delete_number ? _node_revision_delete_time_string('minimum_age_to_delete', $minimum_age_to_delete_number) : $this->t('None');

        // When to delete time (is a number, 0 for always).
        $when_to_delete_number = $node_revision_delete_track[$content_type->id()]['when_to_delete'];
        $when_to_delete = (bool) $when_to_delete_number ? _node_revision_delete_time_string('when_to_delete', $when_to_delete_number) : $this->t('Always delete');

        // Number of candidates nodes to delete theirs revision.
        $candidate_nodes = count(_node_revision_delete_candidates($content_type->id(), $minimum_revisions_to_keep, $minimum_age_to_delete_number, $when_to_delete_number));
        // If we have candidates nodes then we will allow to run the batch job.
        if ($candidate_nodes && !$exists_candidates_nodes) {
          $exists_candidates_nodes = TRUE;
        }

        $route_parameters = [
          'content_type' => $content_type->id(),
        ];
        // Action to delete the configuration for the content type.
        $dropbutton['#links']['delete'] = [
          'title' => $this->t('Untrack'),
          'url' => Url::fromRoute('node_revision_delete.content_type_configuration_delete_confirm', $route_parameters),
        ];
      }
      else {
        $minimum_revisions_to_keep = $this->t('Untracked');
        $candidate_nodes = '-';
        $when_to_delete = $this->t('Untracked');
        $minimum_age_to_delete = $this->t('Untracked');
      }

      // Rendering the dropdown.
      $dropbutton = $this->renderer->render($dropbutton);
      // Setting the row values.
      $rows[] = [
        $content_type->label(),
        $content_type->id(),
        $minimum_revisions_to_keep,
        $minimum_age_to_delete,
        $when_to_delete,
        $candidate_nodes,
        $dropbutton,
      ];
    }
    // Table with current configuration.
    $form['current_configuration'] = [
      '#type' => 'table',
      '#caption' => $this->t('Current configuration'),
      '#header' => $header,
      '#rows' => $rows,
    ];
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
    $form['node_revision_delete_cron'] = [
      '#type' => 'select',
      '#title' => $this->t('How many revisions do you want to delete per cron run?'),
      '#description' => $this->t('Deleting node revisions is a database intensive task. Increase this value if you think that the server can handle more deletions per cron run.'),
      '#options' => $options_node_revision_delete_cron,
      '#default_value' => $config->get('node_revision_delete_cron'),
    ];

    // Available options for node_revision_delete_time variable.
    $options_node_revision_delete_time = [
      'never' => $this->t('Never'),
      'every_time' => $this->t('Every time cron runs'),
      'everyday' => $this->t('Everyday'),
      'every_week' => $this->t('Every Week'),
      'every_10_days' => $this->t('Every 10 Days'),
      'every_15_days' => $this->t('Every 15 Days'),
      'every_month' => $this->t('Every Month'),
      'every_3_months' => $this->t('Every 3 Months'),
      'every_6_months' => $this->t('Every 6 Months'),
      'every_year' => $this->t('Every Year'),
      'every_2_years' => $this->t('Every 2 Years'),
    ];
    $form['node_revision_delete_time'] = [
      '#type' => 'select',
      '#title' => $this->t('How often should revision be deleted while cron runs?'),
      '#description' => $this->t('Frequency of the scheduled mass revision deletion.'),
      '#options' => $options_node_revision_delete_time,
      '#default_value' => $config->get('node_revision_delete_time'),
    ];
    // Time options.
    $allowed_time = [
      'days' => $this->t('Days'),
      'weeks' => $this->t('Weeks'),
      'months' => $this->t('Months'),
    ];
    // Configuration for the node_revision_delete_minimum_age_to_delete_time
    // variable.
    $form['minimum_age_to_delete'] = [
      '#type' => 'fieldset',
      '#title' => $this->t('Minimum age of revision to delete configuration'),
    ];

    $form['minimum_age_to_delete']['node_revision_delete_minimum_age_to_delete_time_max_number'] = [
      '#type' => 'number',
      '#title' => $this->t('Maximum number allowed'),
      '#description' => $this->t('The maximum number in the "Minimum age of revision to delete" configuration in each content type edit page. If you change this number and the new value is smaller than the value defined for a content type in the "Minimum age of revision to delete" setting, the "Minimum age of revision to delete" setting for that content type will take it.'),
      '#default_value' => $config->get('node_revision_delete_minimum_age_to_delete_time')['max_number'],
      '#min' => 1,
    ];

    $form['minimum_age_to_delete']['node_revision_delete_minimum_age_to_delete_time_time'] = [
      '#type' => 'select',
      '#title' => $this->t('The time value'),
      '#description' => $this->t('The time value allowed in the "Minimum age of revision to delete" configuration in each content type edit page. If you change this value all the configured content types will take it.'),
      '#options' => $allowed_time,
      '#size' => 1,
      '#default_value' => $config->get('node_revision_delete_minimum_age_to_delete_time')['time'],
    ];

    // Configuration for the node_revision_delete_when_to_delete_time variable.
    $form['when_to_delete'] = [
      '#type' => 'fieldset',
      '#title' => $this->t('When to delete configuration'),
    ];

    $form['when_to_delete']['node_revision_delete_when_to_delete_time_max_number'] = [
      '#type' => 'number',
      '#title' => $this->t('Maximum number allowed'),
      '#description' => $this->t('The maximum number allowed in the "When to delete" configuration in each content type edit page. If you change this number and the new value is smaller than the value defined for a content type in the "When to delete" setting, the "When to delete" setting for that content type will take it.'),
      '#default_value' => $config->get('node_revision_delete_when_to_delete_time')['max_number'],
      '#min' => 1,
    ];

    $form['when_to_delete']['node_revision_delete_when_to_delete_time_time'] = [
      '#type' => 'select',
      '#title' => $this->t('The time value'),
      '#description' => $this->t('The time value allowed in the "When to delete" configuration in each content type edit page. If you change this value all the configured content types will take it.'),
      '#options' => $allowed_time,
      '#size' => 1,
      '#default_value' => $config->get('node_revision_delete_when_to_delete_time')['time'],
    ];

    // Providing the option to run now the batch job.
    if ($exists_candidates_nodes) {
      $disabled = FALSE;
      $description = $this->t('This will start a batch job to delete old revisions for tracked content types.');
    }
    else {
      $disabled = TRUE;
      $description = $this->t('There not exists candidates nodes with revisions to delete.');
    }

    $form['run_now'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Delete revisions now'),
      '#description' => $description,
      '#disabled' => $disabled,
    ];

    $form['dry_run'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Dry run'),
      '#description' => $this->t('Test run without deleting revisions but seeing the output results.'),
      '#states' => [
        // Hide the dry run option when the run now checkbox is disabled.
        'visible' => [
          ':input[name="run_now"]' => ['checked' => TRUE],
        ],
        // Uncheck if the run_now checkbox is unchecked.
        'unchecked' => [
          ':input[name="run_now"]' => ['checked' => TRUE],
        ],
      ],
    ];

    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    parent::validateForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    parent::submitForm($form, $form_state);
    // Getting the values for node_revision_delete_when_to_delete_time.
    $when_to_delete_time_max_number = $form_state->getValue('node_revision_delete_when_to_delete_time_max_number');
    $node_revision_delete_when_to_delete_time = [
      'max_number' => $when_to_delete_time_max_number,
      'time' => $form_state->getValue('node_revision_delete_when_to_delete_time_time'),
    ];
    // Getting the values for node_revision_delete_minimum_age_to_delete_time.
    $minimum_age_to_delete_time_max_number = $form_state->getValue('node_revision_delete_minimum_age_to_delete_time_max_number');
    $node_revision_delete_minimum_age_to_delete_time = [
      'max_number' => $minimum_age_to_delete_time_max_number,
      'time' => $form_state->getValue('node_revision_delete_minimum_age_to_delete_time_time'),
    ];
    // Saving the configuration.
    _node_revision_delete_update_time_max_number_config('when_to_delete', $when_to_delete_time_max_number);
    _node_revision_delete_update_time_max_number_config('minimum_age_to_delete', $minimum_age_to_delete_time_max_number);
    $this->config('node_revision_delete.settings')
      ->set('node_revision_delete_cron', $form_state->getValue('node_revision_delete_cron'))
      ->set('node_revision_delete_time', $form_state->getValue('node_revision_delete_time'))
      ->set('node_revision_delete_when_to_delete_time', $node_revision_delete_when_to_delete_time)
      ->set('node_revision_delete_minimum_age_to_delete_time', $node_revision_delete_minimum_age_to_delete_time)
      ->save();
  }

}
