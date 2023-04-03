<?php

/**
 * @file
 * Contains lindat.tokens.inc.
 *
 * This file provides lindat tokens.
 */

use Drupal\Core\Render\BubbleableMetadata;
use Drupal\media\Entity\Media;
use Drupal\file\Entity\File;
use Drupal\Core\Datetime\DrupalDateTime;

/**
 * Implements hook_token_info().
 */
function lindat_token_token_info()
{
	$type = [
		'name' => t('Lindat Tokens'),
		'description' => t('Tokens for Lindat.'),
	];

	$node['media_urls'] = [
		'name' => t("Media Urls"),
		'description' => t('URLs of related media files'),
	];

	return [
		'types' => ['lindat' => $type],
		'tokens' => ['lindat' => $node],
	];
}

/**
 * Implements hook_tokens().
 */
function lindat_token_tokens($type, $tokens, array $data, array $options, BubbleableMetadata $bubbleable_metadata)
{
	$replacements = [];
	if ($type == 'lindat' && !empty($data['node'])) {
		foreach ($tokens as $name => $original) {
			switch ($name) {
			case 'media_urls':
				// '^' is used because of a comma hack in 'web/modules/contrib/metatag/src/Plugin/metatag/Tag/MetaNameBase.php'
				$replacements[$original] = ' ' . lindat_url_to_media($data['node'], '^');
				break;
			}
		}
	}

	return $replacements;
}

/**
 * Return machine name of field containing data
 *
 * @param string $bundle
 *   Name of bundle
 *
 * @return string
 *   String with name of field containing data
 *
 * @throws
 *   Throws exeption, when lookup fails
 */
function lindat_get_field_name($bundle)
{
	$field = 'field_media_';
	switch ($bundle) {
/*	case 'image':
		$field .= 'image';
		break;
	case 'book_cover':
		$field .= 'image';
		break;
	case 'illustration':
		$field .= 'image';
		break;
	case 'page_scan':
		$field .= 'image';
		break;
	case 'page_thumbnail':
		$field .= 'image';
		break; */
	case 'book_pdf':
		$field .= 'file';
		break;
/*
	case 'book_text':
		$field .= 'file';
		break;
	case 'marc_xml':
		$field .= 'file';
		break;
	case 'page_text':
		$field .= 'file';
		break;
	case 'tiff_scan':
		$field .= 'file';
		break;
	case 'extracted_text':
		$field .= 'file';
		break;
	case 'file':
		$field .= 'file';
		break;
	case 'fits_technical_metadata':
		$field .= 'file';
		break;
	case 'audio':
		$field .= 'audio_file';
		break;
	case 'video':
		$field .= 'video_file';
		break;
	case 'document':
		$field .= 'document';
		break;
	case 'remote_video':
		$field .= 'oembed_video';
		break; */
	default:
		throw new Exception("Data field lookup failed for bundle '" . $bundle . "'.");
		break;
	}

	return $field;
}

/**
 * Gets published media files URLs of a given node.
 *
 * @param object $node
 *   A core drupal node object.
 *
 * @param string $delimiter
 *   A delimiter character of the result
 *
 * @return string
 *   The tokenized value for the given data. Multiple entries are separated (default ',')
 */
function lindat_url_to_media($node, $delimiter = ',')
{
	$media_array = array();
	$islandora_utils = \Drupal::service('islandora.utils');
	$media_list = $islandora_utils->getMedia($node);

	// finds existing published media and lists their download URLs
	foreach($media_list as $element) {
		if (!is_object($element) || is_null($element->id())) {
			continue;
		}

		$media = Media::load($element->id());

		// skip if not published
		if ($media->get('status')->value != 1) {
			continue;
		}

		// get file field value, catch possible lookup exception
		try {
			$fid = $media->get(lindat_get_field_name($media->bundle()))->getValue();
		} catch(Exception $e) {
			\Drupal::logger('lindat_token')->error($e->getMessage());
			continue;
		}

		$fid_value = (is_array($fid) && array_key_exists(0, $fid) &&
			     array_key_exists('target_id', $fid[0])) ?
			     $fid[0]['target_id'] : NULL;

		if (is_null($fid_value)) {
			continue;
		}

		$file = File::load($fid_value);
		if ($file) {
			array_push($media_array, $islandora_utils->getDownloadUrl($file));
		}
	}

	return implode($delimiter, $media_array);
}
