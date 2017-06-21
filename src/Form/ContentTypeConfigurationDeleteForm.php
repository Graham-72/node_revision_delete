<?php

namespace Drupal\node_revision_delete\Form;

use Drupal\Core\Form\ConfirmFormBase;
use Drupal\Core\Form\FormStateInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\Core\Url;
use Drupal\Core\Entity\EntityManagerInterface;

/**
 * Provides a content type configuration deletion confirmation form.
 */
class ContentTypeConfigurationDeleteForm extends ConfirmFormBase {

  /**
   * Drupal\Core\Entity\EntityManagerInterface definition.
   *
   * @var Drupal\Core\Entity\EntityManagerInterface
   */
  protected $entityManager;

  /**
   * Drupal\Core\Entity\EntityInterface definition.
   *
   * @var \Drupal\Core\Entity\EntityInterface
   */
  protected $contentType;

  /**
   * {@inheritdoc}
   */
  public function __construct(EntityManagerInterface $entity_manager) {
    $this->entityManager = $entity_manager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static($container->get('entity.manager'));
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'content_type_configuration_delete_confirm_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state, $content_type = NULL) {
    $this->contentType = $this->entityManager->getStorage('node_type')->load($content_type);
    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function getQuestion() {
    return $this->t('Are you sure you want to delete the configuration for the "%content_type" content type?', ['%content_type' => $this->contentType->label()]);
  }

  /**
   * {@inheritdoc}
   */
  public function getConfirmText() {
    return $this->t('Delete');
  }

  /**
   * {@inheritdoc}
   */
  public function getDescription() {
    $description = '<p>' . $this->t('This action will delete the Node Revision Delete configuration for the "@content_type" content type, if this action take place the content type will not be available for revision deletion.', ['@content_type' => $this->contentType->label()]) . '</p>';
    $description .= '<p>' . parent::getDescription() . '</p>';
    return $description;
  }

  /**
   * {@inheritdoc}
   */
  public function getCancelText() {
    return $this->t('Cancel');
  }

  /**
   * {@inheritdoc}
   */
  public function getCancelUrl() {
    return new Url('node_revision_delete.admin_settings');
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    // Deleting the content type configuration.
    _node_revision_delete_delete_content_type_config($this->contentType->id());
    // Printing a confirmation message.
    drupal_set_message($this->t('The Node Revision Delete configuration for the "@content_type" content type has been deleted.', ['@content_type' => $this->contentType->label()]));
    // Redirecting.
    $form_state->setRedirectUrl($this->getCancelUrl());
  }

}
