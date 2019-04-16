/*
 * Copyright (C) 2008-2016 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/i2c.h>
#include <sound/soc.h>

static int imx_sph0645_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	u32 channels = 2; //ALWAYS 2 CHANNELS  params_channels(params);
	u32 rate = params_rate(params); //sampling rate
        dev_err(cpu_dai->dev, "sampling rate parms_rate output rate : %d \n" , rate);
	u32 bclk = rate * channels * 32; //fixed to sampling rate * 64
        dev_err(cpu_dai->dev, "bclk : %d \n" , bclk);	
	int ret = 0;

	/* set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret) {
		dev_err(cpu_dai->dev, "failed to set dai fmt\n");
		return ret;
	}

	ret = snd_soc_dai_set_tdm_slot(cpu_dai, 0, 3, 2, 32);
	if (ret) {
		dev_err(cpu_dai->dev, "failed to set dai tdm slot\n");
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, 1, bclk, SND_SOC_CLOCK_OUT);
	if (ret)
		dev_err(cpu_dai->dev, "failed to set cpu sysclk\n");

	return ret;
}

static struct snd_soc_ops imx_sph0645_ops = {
	.hw_params = imx_sph0645_hw_params,
};

static struct snd_soc_dai_link imx_dai = {
	.name = "imx-sph0645",
	.stream_name = "imx-sph0645",
	.codec_name = "snd-soc-dummy",
	.codec_dai_name = "snd-soc-dummy-dai",
	.ops = &imx_sph0645_ops,
	.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS,
	.dpcm_capture = 1,
	.dpcm_playback = 0,
};

static struct snd_soc_card snd_soc_card_imx_3stack = {
	.name = "imx-audio-sph0645",
	.dai_link = &imx_dai,
	.num_links = 1,
	.owner = THIS_MODULE,
};

static int imx_sph0645_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_card_imx_3stack;
	struct device_node *cpu_np, *np = pdev->dev.of_node;
	struct platform_device *cpu_pdev;
	int ret;

	cpu_np = of_parse_phandle(pdev->dev.of_node, "cpu-dai", 0);
	if (!cpu_np) {
		dev_err(&pdev->dev, "phandle missing or invalid\n");
		return -EINVAL;
	}

	cpu_pdev = of_find_device_by_node(cpu_np);
	if (!cpu_pdev) {
		dev_err(&pdev->dev, "failed to find SAI platform device\n");
		ret = -EINVAL;
		goto end;
	}

	card->dev = &pdev->dev;
	card->dai_link->cpu_dai_name = dev_name(&cpu_pdev->dev);
	card->dai_link->platform_of_node = cpu_np;

	platform_set_drvdata(pdev, card);

	ret = snd_soc_register_card(card);
	if (ret)
		dev_err(&pdev->dev, "Failed to register card: %d\n", ret);

end:
	if (cpu_np)
		of_node_put(cpu_np);

	return ret;
}

static int imx_sph0645_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_card_imx_3stack;

	snd_soc_unregister_card(card);

	return 0;
}

static const struct of_device_id imx_sph0645_dt_ids[] = {
	{ .compatible = "fsl,imx-audio-sph0645", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, imx_sph0645_dt_ids);

static struct platform_driver imx_sph0645_driver = {
	.driver = {
		.name = "imx-mic-sph0645",
		.pm = &snd_soc_pm_ops,
		.of_match_table = imx_sph0645_dt_ids,
	},
	.probe = imx_sph0645_probe,
	.remove = imx_sph0645_remove,
};

module_platform_driver(imx_sph0645_driver);

/* Module information */
MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("ALSA SoC i.MX sph0645");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:imx-mic-sph0645");
