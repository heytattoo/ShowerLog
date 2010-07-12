#ifndef ShowerLog_h
#define ShowerLoh_h

#define SHOWERLOG_SIZE 10  // [0<x<256] number of entries which can be stored in the shower log
#define SHOWERLOG_EMPTYVALUE 255 // Should be less than 256, but higher than any possible avg temperature
#define TEMPLOG_SIZE 60 
#define TEMPLOG_EMPTYVALUE 255.0


#include "WProgram.h"

typedef struct
{
	uint8_t temp;
	uint8_t duration;
	uint16_t ageInCycles;
} ShowerLogEntry;

class FloatLog {
	public:
		FloatLog(); 
		~FloatLog(); 
		void add(float e); 
		
		float get(uint8_t i); 
		float oldest();
		
		uint8_t size(); 
		uint8_t numFilled();
		
		// Checks that temperature spikes by at least vChange over 'range' samples
		bool isWarming(float vChange, uint8_t range); 	
		
		// Checks that cooling is monotonic over range; does not have abrupt changes (defined by window, wLimit);
		// and drops by at least vChange over the last range samples
		bool isCooling(float vChange, uint8_t range, uint8_t window, float wLimit); 	
														
		
	private:
		float _log[TEMPLOG_SIZE];	
		void moveEntry(uint8_t from, uint8_t to); 
		int8_t isMonotonic(uint8_t start, uint8_t finish);
		int8_t hasFastChange(uint8_t start, uint8_t finish, uint8_t nRecords, float dLimit);
};

class ShowerLog {
  public:
    ShowerLog();
    ~ShowerLog();
    void incrementAll(); 
    void add(uint8_t Temperature, uint8_t duration, uint16_t cycles_since_end = 0); 
    
    uint8_t getTemp(uint8_t i); 
    uint8_t getDuration(uint8_t i); 
    uint16_t getAge(uint8_t i); 
    
    uint8_t size();
		uint8_t numFilled();
		
		uint16_t checksum();
 
  private:
		ShowerLogEntry _log[SHOWERLOG_SIZE];
	  void moveEntry(uint8_t from, uint8_t to); 
		void writeEmptyEntry(uint8_t i); 
		uint16_t sumEntry(uint8_t i);
		uint16_t _checksum;
		void calcChecksum();
	
	};

#endif