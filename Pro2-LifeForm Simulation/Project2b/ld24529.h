#if !(_ld24529_h)
#define _ld24529_h 1

#include <memory>
#include "LifeForm.h"
#include "Init.h"

class ld24529 : public LifeForm {
protected:
	static void initialize(void);
	void spawn(void);
	void hunt(void);
	void startup(void);
	Event* hunt_event;
public:
	ld24529(void);
	~ld24529(void);
	Color my_color(void) const;   // defines LifeForm::my_color
	static SmartPointer<LifeForm> create(void);
	virtual std::string species_name(void) const;
	virtual std::string player_name(void) const;
	virtual Action encounter(const ObjInfo&);
	friend class Initializer<ld24529>;
};


#endif /* !(_ld24529_h) */
