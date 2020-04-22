/* file "keyboard_mouse.c" */

/*
 *  This is a program that creates a virtual keyboard and mouse device
 *  and then reads a keyboard device to implement mapping certain keys
 *  to mouse events.
 *
 *  Written by Chris Wilson.
 *
 *  This file is hereby placed in the public domain by its author.
 */


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>


static void write_packet(int output_fd, struct input_event *to_write);


extern int main(int argc, const char *argv[])
  {
    if (argc != 2)
      {
        fprintf(stderr, "Usage: %s <input-input-device>\n", argv[0]);
        return 1;
      }

    int input_fd = open(argv[1], O_RDONLY);
    if (input_fd < 0)
      {
        fprintf(stderr, "Failed trying to open \"%s\": %s.\n", argv[1],
                strerror(errno));
        return 1;
      }

    int output_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (output_fd < 0)
      {
        fprintf(stderr, "Failed trying to open \"/dev/uinput\": %s.\n",
                strerror(errno));
        return 1;
      }

      {
        int ioctl_result = ioctl(input_fd, EVIOCGRAB, 1);
        if (ioctl_result == -1)
          {
            fprintf(stderr, "Grabbing input device \"%s\" failed: %s.\n",
                    argv[1], strerror(errno));
            return 1;
          }
      }

      {
        unsigned long event_bits[(EV_MAX/(sizeof(long) * 8)) + 1];
        memset(&(event_bits[0]), 0, sizeof(event_bits));
        int ioctl_result =
                ioctl(input_fd, EVIOCGBIT(0, EV_MAX), &(event_bits[0]));
        if (ioctl_result == -1)
          {
            fprintf(stderr, "Getting supported events on \"%s\" failed: %s.\n",
                    argv[1], strerror(errno));
            return 1;
          }

        for (int event_number = 1; event_number <= EV_MAX; ++event_number)
          {
            if (((event_bits[event_number / (sizeof(long) * 8)] >>
                  (event_number % (sizeof(long) * 8))) & 0x1) == 0)
              {
                continue;
              }

            int ioctl_result = ioctl(output_fd, UI_SET_EVBIT, event_number);
            if (ioctl_result == -1)
              {
                fprintf(stderr,
                        "Setting event bit %d on \"/dev/uinput\" failed: %s.\n",
                        event_number, strerror(errno));
                return 1;
              }

            int code_writer;
            switch (event_number)
              {
                case EV_KEY:
                    code_writer = UI_SET_KEYBIT;
                    break;
                case EV_REL:
                    code_writer = UI_SET_RELBIT;
                    break;
                case EV_ABS:
                    code_writer = UI_SET_ABSBIT;
                    break;
                case EV_MSC:
                    code_writer = UI_SET_MSCBIT;
                    break;
                case EV_LED:
                    code_writer = UI_SET_LEDBIT;
                    break;
                case EV_SND:
                    code_writer = UI_SET_SNDBIT;
                    break;
                case EV_FF:
                    code_writer = UI_SET_FFBIT;
                    break;
                case EV_SW:
                    code_writer = UI_SET_SWBIT;
                    break;
                default:
                    continue;
              }

            unsigned long key_bits[(KEY_MAX/(sizeof(long) * 8)) + 1];
            memset(&(key_bits[0]), 0, sizeof(key_bits));
            ioctl_result = ioctl(input_fd, EVIOCGBIT(event_number, KEY_MAX),
                                 &(key_bits[0]));
            if (ioctl_result == -1)
              {
                fprintf(stderr,
                        "Getting supported codes for event %d on \"%s\" failed:"
                        " %s.\n", event_number, argv[1], strerror(errno));
                return 1;
              }

            for (int key_number = 0; key_number <= KEY_MAX; ++key_number)
              {
                if (((key_bits[key_number / (sizeof(long) * 8)] >>
                      (key_number % (sizeof(long) * 8))) & 0x1) == 0)
                  {
                    continue;
                  }

                int ioctl_result = ioctl(output_fd, code_writer, key_number);
                if (ioctl_result == -1)
                  {
                    fprintf(stderr,
                            "Setting key bit %d for event %d on "
                            "\"/dev/uinput\" failed: %s.\n", key_number,
                            event_number, strerror(errno));
                    return 1;
                  }
              }
          }
      }

      {
        int ioctl_result = ioctl(output_fd, UI_SET_EVBIT, EV_KEY);
        if (ioctl_result == -1)
          {
            fprintf(stderr,
                    "Setting event bit %d on \"/dev/uinput\" failed: %s.\n",
                    EV_KEY, strerror(errno));
            return 1;
          }
      }

      {
        int ioctl_result = ioctl(output_fd, UI_SET_KEYBIT, BTN_LEFT);
        if (ioctl_result == -1)
          {
            fprintf(stderr,
                    "Setting key bit %d for event %d on \"/dev/uinput\" failed:"
                    " %s.\n", BTN_LEFT, EV_KEY, strerror(errno));
            return 1;
          }
      }

      {
        int ioctl_result = ioctl(output_fd, UI_SET_KEYBIT, BTN_MIDDLE);
        if (ioctl_result == -1)
          {
            fprintf(stderr,
                    "Setting key bit %d for event %d on \"/dev/uinput\" failed:"
                    " %s.\n", BTN_MIDDLE, EV_KEY, strerror(errno));
            return 1;
          }
      }

      {
        int ioctl_result = ioctl(output_fd, UI_SET_KEYBIT, BTN_RIGHT);
        if (ioctl_result == -1)
          {
            fprintf(stderr,
                    "Setting key bit %d for event %d on \"/dev/uinput\" failed:"
                    " %s.\n", BTN_RIGHT, EV_KEY, strerror(errno));
            return 1;
          }
      }

      {
        int ioctl_result = ioctl(output_fd, UI_SET_EVBIT, EV_REL);
        if (ioctl_result == -1)
          {
            fprintf(stderr,
                    "Setting event bit %d on \"/dev/uinput\" failed: %s.\n",
                    EV_REL, strerror(errno));
            return 1;
          }
      }

      {
        int ioctl_result = ioctl(output_fd, UI_SET_RELBIT, REL_X);
        if (ioctl_result == -1)
          {
            fprintf(stderr,
                    "Setting rel bit %d for event %d on \"/dev/uinput\" failed:"
                    " %s.\n", REL_X, EV_REL, strerror(errno));
            return 1;
          }
      }

      {
        int ioctl_result = ioctl(output_fd, UI_SET_RELBIT, REL_Y);
        if (ioctl_result == -1)
          {
            fprintf(stderr,
                    "Setting rel bit %d for event %d on \"/dev/uinput\" failed:"
                    " %s.\n", REL_Y, EV_REL, strerror(errno));
            return 1;
          }
      }

    struct uinput_setup setup_data;
    memset(&setup_data, 0, sizeof(setup_data));
    setup_data.id.bustype = BUS_USB;
    setup_data.id.vendor = 0xcccc;
    setup_data.id.product = 0xcccc;
    strcpy(setup_data.name, "Keyboard Mouse");

    int ioctl_result = ioctl(output_fd, UI_DEV_SETUP, &setup_data);
    if (ioctl_result == -1)
      {
        fprintf(stderr, "Setup on \"/dev/uinput\" failed: %s.\n",
                strerror(errno));
        return 1;
      }

    ioctl_result = ioctl(output_fd, UI_DEV_CREATE);
    if (ioctl_result == -1)
      {
        fprintf(stderr, "Device creation on \"/dev/uinput\" failed: %s.\n",
                strerror(errno));
        return 1;
      }

    size_t buffer_position = 0;
    int skip_first = 0;
    struct input_event incoming[3];
    int super_held = 0;
    int speed = 10;
    while (1)
      {
        ssize_t read_count = read(input_fd, &(incoming[buffer_position]),
                                  sizeof(struct input_event));
        if (read_count != sizeof(struct input_event))
            break;
        switch (buffer_position)
          {
            case 0:
                if (incoming[0].type == 1)
                  {
                    memcpy(&(incoming[1]), &(incoming[0]),
                           sizeof(struct input_event));
                    buffer_position = 2;
                    skip_first = 1;
                  }
                else if ((incoming[0].type == 4) && (incoming[0].code == 4))
                  {
                    ++buffer_position;
                  }
                else
                  {
                    write_packet(output_fd, &(incoming[0]));
                  }
                break;
            case 1:
                if (incoming[1].type == 1)
                  {
                    ++buffer_position;
                  }
                else
                  {
                    if (!skip_first)
                      {
                        write_packet(output_fd, &(incoming[0]));
                        skip_first = 0;
                      }
                    write_packet(output_fd, &(incoming[1]));
                    buffer_position = 0;
                  }
                break;
            case 2:
                if (incoming[2].type != EV_SYN)
                    goto do_write;
                int x, y;
                int button_code;
                switch (incoming[1].code)
                  {
                    case 126:
                        super_held = (incoming[1].value != 0);
                        break;
                    case 17: /* W */
                        if (super_held)
                          {
                            x = -1;
                            y = -1;
                            goto do_x_y;
                          }
                        break;
                    case 18: /* E */
                        if (super_held)
                          {
                            x = 0;
                            y = -1;
                            goto do_x_y;
                          }
                        break;
                    case 19: /* R */
                        if (super_held)
                          {
                            x = 1;
                            y = -1;
                            goto do_x_y;
                          }
                        break;
                    case 31: /* S */
                        if (super_held)
                          {
                            x = -1;
                            y = 0;
                            goto do_x_y;
                          }
                        break;
                    case 32: /* D */
                        if (super_held)
                          {
                            x = 0;
                            y = 0;
                            goto do_x_y;
                          }
                        break;
                    case 33: /* F */
                        if (super_held)
                          {
                            x = 1;
                            y = 0;
                            goto do_x_y;
                          }
                        break;
                    case 45: /* X */
                        if (super_held)
                          {
                            x = -1;
                            y = 1;
                            goto do_x_y;
                          }
                        break;
                    case 46: /* C */
                        if (super_held)
                          {
                            x = 0;
                            y = 1;
                            goto do_x_y;
                          }
                        break;
                    case 47: /* V */
                        if (super_held)
                          {
                            x = 1;
                            y = 1;
                            goto do_x_y;
                          }
                        break;
                    case 36: /* J */
                        button_code = 272;
                        goto do_button;
                    case 37: /* K */
                        button_code = 274;
                        goto do_button;
                    case 38: /* L */
                        button_code = 273;
                        goto do_button;
                    default:
                        break;
                  }
              do_write:
                if (skip_first)
                    skip_first = 0;
                else
                    write_packet(output_fd, &(incoming[0]));
                write_packet(output_fd, &(incoming[1]));
                write_packet(output_fd, &(incoming[2]));
                buffer_position = 0;
                break;
              do_x_y:
                if (incoming[1].value == 0)
                  {
                    speed = 10;
                    break;
                  }
                if (skip_first)
                  {
                    incoming[0].time = incoming[1].time;
                    skip_first = 0;
                  }
                incoming[0].type = EV_REL;
                incoming[0].code = REL_X;
                incoming[0].value = x * speed;
                incoming[1].type = EV_REL;
                incoming[1].code = REL_Y;
                incoming[1].value = y * speed;
                ++speed;
                goto do_write;
              do_button:
                if (super_held && (incoming[1].value != 2))
                  {
                    int save_value = incoming[1].value;
                    incoming[1].code = 126;
                    incoming[1].value = 0;
                    write_packet(output_fd, &(incoming[1]));
                    write_packet(output_fd, &(incoming[2]));
                    incoming[1].code = button_code;
                    incoming[1].value = save_value;
                    write_packet(output_fd, &(incoming[1]));
                    write_packet(output_fd, &(incoming[2]));
                    incoming[1].code = 126;
                    incoming[1].value = 1;
                    skip_first = 1;
                  }
                goto do_write;
            default:
                assert(0);
          }
      }

    ioctl(output_fd, UI_DEV_DESTROY);
    close(output_fd);
    close(input_fd);
    return 0;
  }


static void write_packet(int output_fd, struct input_event *to_write)
  {
    int result = write(output_fd, to_write, sizeof(struct input_event));
    if (result != sizeof(struct input_event))
      {
        fprintf(stderr, "Writing to \"/dev/uinput\" failed: %s.\n",
                strerror(errno));
        exit(1);
      }
  }
