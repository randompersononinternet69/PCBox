/* Copyright holders: Sarah Walker
   see COPYING for more details
*/
#ifndef EMU_GAMEPORT_H
# define EMU_GAMEPORT_H


#define AXIS_NOT_PRESENT -99999


typedef struct
{
        const char *name;
        void *(*init)(void);
        void (*close)(void *p);
        uint8_t (*read)(void *p);
        void (*write)(void *p);
        int (*read_axis)(void *p, int axis);
        void (*a0_over)(void *p);
        int axis_count, button_count, pov_count;
        int max_joysticks;
        const char *axis_names[8];
        const char *button_names[32];
        const char *pov_names[4];
} joystick_if_t;



#ifdef __cplusplus
extern "C" {
#endif

extern device_t gameport_device;
extern device_t gameport_201_device;

extern int joystick_type;


extern char	*joystick_get_name(int64_t joystick);
extern int64_t	joystick_get_max_joysticks(int64_t joystick);
extern int64_t	joystick_get_axis_count(int64_t joystick);
extern int64_t	joystick_get_button_count(int64_t joystick);
extern int64_t	joystick_get_pov_count(int64_t joystick);
extern char	*joystick_get_axis_name(int64_t joystick, int64_t id);
extern char	*joystick_get_button_name(int64_t joystick, int64_t id);
extern char	*joystick_get_pov_name(int64_t joystick, int64_t id);

extern void	gameport_update_joystick_type(void);

#ifdef __cplusplus
}
#endif


#endif	/*EMU_GAMEPORT_H*/
