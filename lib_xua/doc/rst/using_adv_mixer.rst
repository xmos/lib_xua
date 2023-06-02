|newpage|

Mixer
=====

Since the mixer has no I/O the instantiation is straight forward. Communication wises, the mixer cores are inserted 
between the `AudioHub` and Buffering core(s)

It takes three channel ends as parameters, one for audio to/from the buffering core(s), one for audio to/from the 
`AudioHub` core and another one for control requests from the `Endpoint0` core.

The mixer task will automatically handle the change in mix count based on the current sample frequency (communicated
via the data channels from the buffering task).

An example of how the mixer task might be called is shown below (some parameter lists are abbreviated) ::

    chan c_aud_0, c_aud_1, c_mix_ctl;
    
    par
    {
        XUA_Buffer(..., c_aud_0, ...);

        mixer(c_aud0, c_aud_1, c_mix_ctl);

        XUA_AudioHub(c_aud_1, ...);

        XUA_Endpoint0(..., c_mix_ctl, ...);
    }
