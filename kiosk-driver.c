#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>


static unsigned int redLED = 26;
static unsigned int yellowLED = 5;
static unsigned int greenLED = 12;
static unsigned int buzzer = 13;        
static unsigned int gpioButton = 6;   
static unsigned int irqNumber;
static unsigned int feedbackState = 0;
static unsigned int buttonOnCooldown = 0;
struct timer_list cooldown_timer;

//Quick and dirty way of checking all the pins
static int check_pins_valid(void){
    int count = 0;
    count = count + gpio_is_valid(gpioButton);
    count = count + gpio_is_valid(buzzer);
    count = count + gpio_is_valid(greenLED);
    count = count + gpio_is_valid(yellowLED);
    count = count + gpio_is_valid(redLED);

    if (count == 5)
    {
        return 1;
    }
    return 0;
}

static void free_pins(void){
    if (gpio_is_valid(gpioButton)) {
        gpio_free(gpioButton);
    }
    if (gpio_is_valid(buzzer)) {
        gpio_free(buzzer);
    }
    if (gpio_is_valid(greenLED)) {
        gpio_free(greenLED);
    }
    if (gpio_is_valid(yellowLED)) {
        gpio_free(yellowLED);
    }
    if (gpio_is_valid(redLED)) {
        gpio_free(redLED);
    }
}

static int claim_pins(void)
{
    int result = 0;
    result = gpio_request(redLED, "redLED");
    if (result < 0) {
        printk(KERN_ERR "Failed to request redLED pin\n");
        return result;
    }
    result = gpio_request(yellowLED, "yellowLED");
    if (result < 0) {
        printk(KERN_ERR "Failed to request yellowLED pin\n");
        gpio_free(redLED);
        return result;
    }
    result = gpio_request(greenLED, "greenLED");
    if (result < 0) {
        printk(KERN_ERR "Failed to request greenLED pin\n");
        gpio_free(redLED);
        gpio_free(yellowLED);
        return result;
    }
    result = gpio_request(buzzer, "buzzer");
    if (result < 0) {
        printk(KERN_ERR "Failed to request buzzer pin\n");
        gpio_free(redLED);
        gpio_free(yellowLED);
        gpio_free(greenLED);
        return result;
    }
    result = gpio_request(gpioButton, "gpioButton");
    if (result < 0) {
        printk(KERN_ERR "Failed to request gpioButton pin\n");
        gpio_free(redLED);
        gpio_free(yellowLED);
        gpio_free(greenLED);
        gpio_free(buzzer);
        return result;
    }

    gpio_direction_output(redLED, 0);
    gpio_direction_output(yellowLED, 0);
    gpio_direction_output(greenLED, 0);
    gpio_direction_output(buzzer, 0);
    gpio_direction_input(gpioButton);

    return 0;
}

//Turn on one of the 3 LEDs based on input value
static void set_leds(unsigned int led_value) {
    gpio_set_value(redLED, led_value == 0 ? 1 : 0);
    gpio_set_value(yellowLED, led_value == 1 ? 1 : 0);
    gpio_set_value(greenLED, led_value == 2 ? 1 : 0);
}

static void reset_state(void){
    gpio_set_value(redLED, 0);
    gpio_set_value(yellowLED, 0);
    gpio_set_value(greenLED, 0);
    feedbackState = 0;
}

void cooldown_timer_callback(struct timer_list *t)
{
    printk("Sending to userspace the value : %d\n", feedbackState);
    gpio_set_value(buzzer, 1);
    mdelay(50);
    gpio_set_value(buzzer, 0);
    mdelay(50);
    gpio_set_value(buzzer, 1);
    mdelay(50);
    gpio_set_value(buzzer, 0);
    mdelay(50);
    gpio_set_value(buzzer, 1);
    mdelay(50);
    gpio_set_value(buzzer, 0);
    mdelay(50);
    reset_state();
}

static irq_handler_t kioskbtn_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) { 
    printk("Button pressed\n");
    //A more intuitive debouncer than certain other solutions online
    if (buttonOnCooldown)
    {
        printk("Button pressed before cooldown finished\n");
        return (irq_handler_t) IRQ_HANDLED;
    }
    del_timer(&cooldown_timer);
    
    feedbackState = (feedbackState + 1) % 3;
    set_leds(feedbackState);
    buttonOnCooldown = 1;
    gpio_set_value(buzzer, 1);
    mdelay(50);
    gpio_set_value(buzzer, 0);
    mdelay(500);
    buttonOnCooldown = 0;
    mod_timer(&cooldown_timer, jiffies + 5 * HZ);
    return (irq_handler_t) IRQ_HANDLED;
}


static int __init kiosk_driver_init(void)
{
    int result = 0;
    printk("Kiosk Driver init begin\n");
    if (check_pins_valid())
    {
        printk("Valid GPIO pins set\n");

        if (claim_pins() == 0)
        {
            printk("Claimed all pins successfully\n");
        }
        
    }
    else{
        printk("Invalid GPIO pins set\nExiting\n");
        return -ENODEV;
    }

    irqNumber = gpio_to_irq(gpioButton);

    result = request_irq(irqNumber, (irq_handler_t)kioskbtn_irq_handler, IRQF_TRIGGER_RISING, "kiosk_gpio_button_handler", NULL);

    if (result)
    {
        printk(KERN_ERR "Failed to request IRQ for GPIO button\n");
        return result;
    }
    timer_setup(&cooldown_timer, cooldown_timer_callback, 0);

    return 0;
}

static void __exit kiosk_driver_exit(void)
{
    free_pins();
    free_irq(irqNumber, NULL);
    del_timer(&cooldown_timer);
	printk("Leaving my driver!\n");
	return;
}

module_init(kiosk_driver_init);
module_exit(kiosk_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("1902765@uad.ac.uk");
MODULE_DESCRIPTION("Driver for control over the mini Kiosk");
MODULE_VERSION("1.0");