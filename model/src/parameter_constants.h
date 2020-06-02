/*
 * ParameterConstants.h
 *
 *  Created on: Jul 22, 2016
 *      Author: nick
 */

#ifndef PARAMETER_CONSTANTS_H_
#define PARAMETER_CONSTANTS_H_

namespace crx {

const std::string PERSONS_FILE = "persons.file";
const std::string PERSONS_ALPHA_FILE = "persons.alpha.file";
const std::string PLACES_FILE = "places.file";
const std::string CLINICS_FILE = "clinics.file";
const std::string ACTIVITIES_FILE = "activities.file";
const std::string BUCKETS_FILE = "buckets.file";
const std::string SCHEDULE_FILE = "schedule.file";
const std::string HRX_FILE = "hrx.file";

const std::string PLACES_SERVICE_CODES = "places.service.codes.file";
const std::string SERVICE_CODES = "service.codes.file";
const std::string SERVICE_DELTAS_FILE = "service.deltas.file";

const std::string INIT_PLACES_FILE = "initial.places";
const std::string INIT_CLINICS_FILE = "initial.clinics";

const std::string GAMMA_LOW = "gamma.low";
const std::string GAMMA_MED = "gamma.med";
const std::string GAMMA_HIGH = "gamma.high";

const std::string DELTA_MULTIPLIER = "delta.multiplier";
const std::string DELTA_LOW = "delta.low";
const std::string DELTA_MED = "delta.med";
const std::string DELTA_HIGH = "delta.high";

const std::string DOSING_DECAY = "dosing.decay";
const std::string DOSING_DOCTOR = "dosing.doctor";
const std::string DOSING_PSR = "dosing.psr";
const std::string DOSING_NURSE = "dosing.nurse";
const std::string DOSING_RESOURCE = "dosing.resource";
const std::string DOSING_PEER = "dosing.peer";
const std::string DOSING_KAPPA = "dosing.kappa";

const std::string OUTPUT_DIRECTORY = "output.directory";
const std::string BETA_OUTPUT_FILE = "beta.output.file";
const std::string CHOICE_OUTPUT_FILE = "choice.output.file";
const std::string VISIT_OUTPUT_FILE = "visit.output.file";
const std::string INFO_NET_OUTPUT_FILE = "info.net.output.file";
const std::string PLACE_NET_OUTPUT_FILE = "place.net.output.file";
const std::string HRX_SHARE_OUTPUT_FILE = "hrx.shared.output.file";
const std::string HRX_RECV_OUTPUT_FILE = "hrx.recv.output.file";
const std::string HRX_TAG_COUNTS_FILE = "hrx.tag.counts.file";

const std::string CHOICE_SERVICE_CODES = "choice.recording.service.codes";

const std::string VISIT_RECORDING_ATUS_CODES = "visit.recording.atus.codes";
const std::string VISIT_RECORDING_SERVICE_CODES = "visit.recording.service.codes";
const std::string VISIT_RECORDING_PERSONS = "visit.recording.persons";
const std::string VISIT_RECORDING_PLACES = "visit.recording.places";

const std::string BETA_RECORDING_FREQUENCY = "beta.recording.frequency";
const std::string BETA_RECORDING_PERSONS = "beta.recording.persons";

const std::string DOSING_OUTPUT_FILE = "dosing.output.file";
const std::string DOSING_RECORDING_PERSONS = "dosing.recording.persons";
const std::string DOSING_RECORDING_CAUSES = "dosing.recording.causes";

const std::string PROPENSITY_FILE = "propensity.file";

const std::string PROPENSITY_MULTIPLIER = "propensity.multiplier";
const std::string PROPENSITY_NONE = "propensity.none";
const std::string PROPENSITY_LOW = "propensity.low";
const std::string PROPENSITY_MED = "propensity.medium";
const std::string PROPENSITY_HIGH = "propensity.high";

const std::string UNCERTAINTY_LOW = "uncertainity.low";
const std::string UNCERTAINTY_MED = "uncertainity.medium";
const std::string UNCERTAINTY_HIGH = "uncertainity.high";

}



#endif /* PARAMETER_CONSTANTS_H_ */
