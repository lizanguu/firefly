int main(int argc, char *argv[])
{
    if (htk_init_shell(argc, argv, hcompv_version, hcompv_vc_id) < HTK_OK)
      htk_error(2000, "htk_compv: init shell failed");

    htk_init_mem();
    htk_init_label();
    htk_init_math();
    htk_init_sigp();
    htk_init_wave();
    htk_init_audio();
    htk_init_vq();
    htk_init_model();

    if (!htk_info_printed() && htk_num_args() == 0)
      htk_report_usage();

    path_pattern[0] = '\0';
    while (htk_next_arg() == HTK_SWITCH_ARG)
    {
        s = htk_get_swt_arg();
        if (strlen(s) != 1)
          htk_error(2019, "htk_compv: bad switch %s; must be single letter", s);
        switch (s[0])
        {
            case 'f':
                if (htk_next_arg() != HTK_FLOAT_ARG)
                  htk_error(2019, "htk_compv: variance floor scale expected");
                vfloor_scale = htk_get_chked_flt(0.0, 100.0, s);
                break;
            case 'l':
                if (htk_next_arg() != HTK_STRING_ARG)
                  htk_error(2019, "htk_compv: segment label expected");
                seglab = htk_get_str_arg();
                break;
            case 'm':
                mean_update = HTK_TRUE;
                break;
            case 'o':
                outfn = htk_get_str_arg();
                break;
            case 'v':
                if (htk_next_arg() != HTK_FLOAT_ARG)
                  htk_error(2019, "htk_compv: minimum variance level expected");
                min_var = htk_get_chked_flt();
                break;
            case 'k':
                if (htk_next_arg() != HTK_STRING_ARG)
                  htk_error(2019, "htk_compv: speaker pattern expected");
                strcpy(sp_pattern, htk_get_str_arg());
                if (strchr(sp_pattern, '%') == NULL)
                  htk_error(2019, "htk_compv: speaker mask invalid");
                break;
        }
    }
}
