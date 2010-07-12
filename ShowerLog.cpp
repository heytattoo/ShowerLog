
#include "WProgram.h"
#include "ShowerLog.h"
#define TRUE 1
#define FALSE 0

////////////////////////////////////////////
////////////// CLASS : FloatLog ////////////
////////////////////////////////////////////

//
// Returns TRUE if the change is abrupt enough at the start of the log
// Returns FALSE if it ain't.
//
bool FloatLog::isWarming(float vChange, uint8_t range){
	if ((_log[0] > _log[range]) && hasFastChange(0,range,range+1,vChange)) return TRUE;
	return FALSE;
}

//
// Tests for monotonic range from _log[start] to _log[finish], inclusive.
// If monotonic, returns 2
// If constant, returns 1
// If non-monotonic, returns 0
// If input error, returns -1
//
int8_t FloatLog::isMonotonic(uint8_t start, uint8_t finish){
	if (finish <= start) return -1;	// start must be less than finish
	bool dirKnown = FALSE;
	bool isInc = FALSE; // unknown at this point
	float d;
	
	for (uint8_t i = start; i<finish; i++){
		d = _log[i+1] - _log[i];
		if (!dirKnown) {
			// Try to figure out what direction this thing is moving
			if (d > 0){
				isInc = TRUE;
				dirKnown = TRUE;
			} else if (d<0){
				isInc = FALSE;
				dirKnown = TRUE;
			}
		} else {
			// direction is known; check that this d is consistent
			if ((d>0) && !isInc) return 0;
			if ((d<0) && isInc) return 0;
		}
	}
	// got through loop; no inconsistencies
	if (dirKnown) return 2; // range is monotonic, in one direction or the other 
	else return 1; // range is constant
}

int8_t FloatLog::hasFastChange(uint8_t start, uint8_t finish, uint8_t nRecords, float dLimit){
	// dLimit is the limit in absolute change in value over a range of nRecords
	// for example, if dlimit=1.0 and nRecords = 3, then a section in the start:finish range of
	// _log of [32, 32.5, 33] would return 1.
	// a section of [32, 32.5, 32.5, 33] would return a 0
	// a section of [32, 32.5, 32] would return a 0
	// a section of [32, 33, 32] would return a 1
	// a section of [32, 31.5, 32.5] would return a 0 <---- IMPORTANT
	// 
	// start must be < finish, else -1 is returned
	// if finish - start + 1 < nRecords, -1 is returned
	if (start >= finish) return -1; // input error
	if ((finish + 1 - start) < nRecords) return -1;
	float d;
	uint8_t j;
	
	for (uint8_t i = start; i<=(finish + 1 - nRecords); i++){
		// check the first section
		d = 0;
		for (j = i; j<(i+nRecords-2); j++){
			d += _log[j+1]-_log[i]; // change in value
			if ((d >= dLimit) || (d <= -1*dLimit)) return 1;
		} 
	}
	return 0;
}

//
// Returns the index of the most recent falling temperature edge
//
bool FloatLog::isCooling(float vChange, uint8_t range, uint8_t window, float wLimit){
	// check for monotonic decrease in temperature in near history
	// trend must be "range" entries long, with a value change of at least "vChange"
	// Also, range cannot include any steep changes in temperature (wLimit degrees over window samples)
	
	// Doh.  No input error checking... 
	
	if ( ((_log[range] - _log[0]) > vChange) && isMonotonic(0,range) && !hasFastChange(0,range,window,wLimit)){
		return TRUE;
	}
	return FALSE;
	
}

//
// Constructor
//
FloatLog::FloatLog(){
	for (uint8_t i = 0; i < TEMPLOG_SIZE; i++) _log[i]=TEMPLOG_EMPTYVALUE; // empty log entries
}

//
// Destructor
//
FloatLog::~FloatLog(){
}

//
// Returns the total size of the log, in number of possible entries
//
uint8_t FloatLog::size(){
	return TEMPLOG_SIZE;
}

// 
// Adds a new entry to position zero.  Bumps the oldest entry 
// out of the log.  Moves all existing entries back by one position.
//
void FloatLog::add(float e){
	for (uint8_t i = (size()-1); i > 0; i--) moveEntry(i-1,i);
	_log[0]=e;
}

//
// Returns the value at position i; 
//
float FloatLog::get(uint8_t i){
	if ((i >= size()) || (i < 0)) return -1.0;
	return _log[i];
}

//
// Returns the number of non-empty entries currently in the log.
// Assumes that once one empty entry is found, all older entries are also
// empty.
//
uint8_t FloatLog::numFilled(){
	uint8_t i;
	for (i = 0; i<size(); i++){
		if (_log[i] == TEMPLOG_EMPTYVALUE) break;
	}
	return i;
}

//
// Returns the value at the end of the log. 
//
float FloatLog::oldest(){
	return _log[size()-1];
}


//
// Moves an existing entry to a new location in the log.  
// Over-writes any existing entry.
//
void FloatLog::moveEntry(uint8_t from, uint8_t to){
	if ((from >= size()) || (from < 0)) _log[to]=666666; 
	if ((to >= size()) || (to < 0)) _log[to]=666666;
	_log[to] = _log[from];
}

/////////////////////////////////////////////
////////////// CLASS : ShowerLog ////////////
/////////////////////////////////////////////

// 
// Adds all values in a single log entry
//
uint16_t ShowerLog::sumEntry(uint8_t i){
	return (uint16_t)_log[i].temp + (uint16_t)_log[i].duration + _log[i].ageInCycles;
}

//
// Adds all values from all entries, and stores sum in a private data member.
// Should be run after any changes are made to the log contents;
//
void ShowerLog::calcChecksum(){
	_checksum = 0;
	for (uint8_t i = 0; i<size(); i++) _checksum+=sumEntry(i);
}

//
// Returns the log's checksum value
//
uint16_t ShowerLog::checksum(){
	return _checksum;
}

//
// Writes an empty entry to a location in the log
//
void ShowerLog::writeEmptyEntry(uint8_t i){
	_log[i].temp = SHOWERLOG_EMPTYVALUE;
	_log[i].duration = SHOWERLOG_EMPTYVALUE;
	_log[i].ageInCycles = SHOWERLOG_EMPTYVALUE;
}

// 
// Adds a new entry to position zero.  Bumps the oldest entry 
// out of the log.  Moves all existing entries back by one position.
//
void ShowerLog::add(uint8_t temperature, uint8_t duration, uint16_t ageInCycles){
	for (uint8_t i = SHOWERLOG_SIZE; i > 0; i--) moveEntry(i-1,i);
	_log[0].temp = temperature;
	_log[0].duration = duration;
	_log[0].ageInCycles = ageInCycles;
	calcChecksum();
}

//
// Constructor
//
ShowerLog::ShowerLog() {
	for (uint8_t i = 0; i < SHOWERLOG_SIZE; i++) writeEmptyEntry(i); // empty log entries
	calcChecksum();
}

//
// Destructor
//
ShowerLog::~ShowerLog() {
}

//
// Moves an existing entry to a new location in the log.  
// Over-writes any existing entry.
//
void ShowerLog::moveEntry(uint8_t from, uint8_t to){
	if ((from >= size()) || (from < 0)) return;
	if ((to >= size()) || (to < 0)) return;
	_log[to] = _log[from];
}

//
// Ages all entries in the log by one cycle.
// WARNING: Does not check for rollover of the uint16_t value
//
void ShowerLog::incrementAll(){
	for (uint8_t i = 0; i < SHOWERLOG_SIZE; i++) _log[i].ageInCycles++;
	calcChecksum();
}

//
// Looks up and returns the averge temperature of a specific entry
//
uint8_t ShowerLog::getTemp(uint8_t i){
	if ((i >= SHOWERLOG_SIZE) || (i < 0)) return 66;
	return (uint8_t)_log[i].temp;
}

//
// Looks up and returns the duration of a specific entry
//
uint8_t ShowerLog::getDuration(uint8_t i){
	if ((i >= SHOWERLOG_SIZE) || (i < 0)) return 66;
	return (uint8_t)_log[i].duration;
}

//
// Looks up and returns the age in cycles of a specific entry
//
uint16_t ShowerLog::getAge(uint8_t i){
	if ((i >= SHOWERLOG_SIZE) || (i < 0)) return 666;
	return _log[i].ageInCycles;
}

//
// Returns the total size of the log
//
uint8_t ShowerLog::size(){
	return (uint8_t)SHOWERLOG_SIZE;
}

//
// Returns the number of non-empty entries currently in the log.
// Assumes that once one empty entry is found, all older entries are also
// empty.
//
uint8_t ShowerLog::numFilled(){
	uint8_t i;
	for (i = 0; i<size(); i++){
		if (_log[i].temp == SHOWERLOG_EMPTYVALUE) break;
	}
	return i;
}
