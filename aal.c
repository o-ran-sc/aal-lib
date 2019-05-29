#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>


#include "aal.h"

/****
* bef: backend function
* bep: backend path
****/
int aal_load_libbe(struct be_func* bef, char* bep)
{
	bef->handle = dlopen(bep, RTLD_LAZY);
	if (bef->handle == NULL)
	{
		printf("AAL dlopen '%s' error.\n", bep);
		goto dlopen_error;
	}

	bef->init = dlsym(bef->handle, "be_init");
	if (bef->init == NULL)
	{
		printf("AAL dlsym 'be_init' func error from '%s'.\n", bep);
		goto dlsym_error;
	}

	bef->read = dlsym(bef->handle, "be_read");
	if (bef->read == NULL)
	{
		printf("AAL dlsym 'be_read' func error from '%s'.\n", bep);
		goto dlsym_error;
	}

	bef->write = dlsym(bef->handle, "be_write");
	if (bef->write == NULL)
	{
		printf("AAL dlsym 'be_write' func error from '%s'.\n", bep);
		goto dlsym_error;
	}

	bef->clean = dlsym(bef->handle, "be_clean");
	if (bef->clean == NULL)
	{
		printf("AAL dlsym 'be_clean' func error from '%s'.\n", bep);
		goto dlsym_error;
	}

	return 0;

dlsym_error:
	
	dlclose(bef->handle);

dlopen_error:
	
	return -1;
}

int aal_unload_libbe(struct be_func* bef)
{
	dlclose(bef->handle);
	return 0;
}

/****
* pt: plugin function
* pp: plugin path
****/
int aal_load_libplugin(struct plugin_func* pf, char* pp)
{
	pf->handle = dlopen(pp, RTLD_LAZY);
	if (pf->handle == NULL)
	{
		printf("AAL dlopen '%s' error.\n", pp);
		goto dlopen_error;
	}

	pf->init = dlsym(pf->handle, "plugin_init");
	if (pf->init == NULL)
	{
		printf("AAL dlsym 'plugin_init' func error from '%s'.\n", pp);
		goto dlsym_error;
	}

	pf->read = dlsym(pf->handle, "plugin_read");
	if (pf->read == NULL)
	{
		printf("AAL dlsym 'plugin_read' func error from '%s'.\n", pp);
		goto dlsym_error;
	}

	pf->write = dlsym(pf->handle, "plugin_write");
	if (pf->write == NULL)
	{
		printf("AAL dlsym 'plugin_write' func error from '%s'.\n", pp);
		goto dlsym_error;
	}

	pf->clean = dlsym(pf->handle, "plugin_clean");
	if (pf->clean == NULL)
	{
		printf("AAL dlsym 'plugin_clean' func error from '%s'.\n", pp);
		goto dlsym_error;
	}
	
	return 0;

dlsym_error:
	
	dlclose(pf->handle);

dlopen_error:
	
	return -1;
}

int aal_unload_libplugin(struct plugin_func* pf)
{
	dlclose(pf->handle);
	return 0;
}

int aal_load_config_be_type(struct aal_config* conf, xmlNodePtr root)
{
	xmlNodePtr be_type;
	xmlNodePtr be_type_data;

	xmlChar* name;
	xmlChar* libpath;

	for (be_type = root->xmlChildrenNode; be_type != NULL; be_type = be_type->next)
	{
		if(!xmlStrcmp(be_type->name, (const xmlChar *)("BE_type")))
		{
			// find BE_type node
			break;
		}
	}

	// 
	if (be_type == NULL)
		return -1;

	for (be_type_data = be_type->xmlChildrenNode; be_type_data != NULL; be_type_data = be_type_data->next)
	{
		if(!xmlStrcmp(be_type_data->name, (const xmlChar *)("data")))
		{
			if ((name = xmlGetProp(be_type_data, "name")) == NULL)
				continue;
			
			if ((libpath = xmlGetProp(be_type_data, "libpath")) == NULL)
				continue;
			
			snprintf(conf->bet[conf->betc].name, AAL_NAME_STRING_MAX_LEN, "%s", name);
			if (aal_load_libbe(&conf->bet[conf->betc].func, libpath) != 0)
			{
				printf("AAL load libbe '%s' error.\n", libpath);
				continue;
			}
			
			conf->betc++;
		}
	}

	if (conf->betc == 0)
	{
		return -1;
	}

	return 0;
}

int aal_load_config_plugin_type(struct aal_config* conf, xmlNodePtr root)
{
	xmlNodePtr plugin_type;
	xmlNodePtr plugin_type_data;

	xmlChar* name;
	xmlChar* libpath;

	for (plugin_type = root->xmlChildrenNode; plugin_type != NULL; plugin_type = plugin_type->next)
	{
		if(!xmlStrcmp(plugin_type->name, (const xmlChar *)("plugin_type")))
		{
			// find plugin_type node
			break;
		}
	}

	// 
	if (plugin_type == NULL)
		return -1;

	for (plugin_type_data = plugin_type->xmlChildrenNode; plugin_type_data != NULL; plugin_type_data = plugin_type_data->next)
	{
		if(!xmlStrcmp(plugin_type_data->name, (const xmlChar *)("data")))
		{
			if ((name = xmlGetProp(plugin_type_data, "name")) == NULL)
				continue;
			
			if ((libpath = xmlGetProp(plugin_type_data, "libpath")) == NULL)
				continue;

			snprintf(conf->pt[conf->ptc].name, AAL_NAME_STRING_MAX_LEN, name);
			if (aal_load_libplugin(&conf->pt[conf->ptc].func, libpath) != 0)
			{
				printf("AAL load libplugin '%s' error.\n", libpath);
				continue;
			}
			
			conf->ptc++;
		}
	}
	
	if (conf->ptc == 0)
	{
		return -1;
	}

	return 0;
}


int aal_be_type_name_to_index(struct aal_config* conf, char* name)
{
	uint32_t i;
	for (i = 0; i < conf->betc; i++)
		if (strcmp(conf->bet[i].name, name) == 0)
			break;

	return i == conf->betc ? 0xff : i;
}

int aal_plugin_type_name_to_index(struct aal_config* conf, char* name)
{
	uint32_t i;
	for (i = 0; i < conf->ptc; i++)
		if (strcmp(conf->pt[i].name, name) == 0)
			break;

	return i == conf->ptc ? 0xff : i;
}


int aal_load_config_be_dev(struct aal_config* conf, xmlNodePtr root)
{
	uint32_t i;
	xmlNodePtr be_dev;
	xmlNodePtr be_dev_data;

	xmlChar* name;
	xmlChar* type;
	uint32_t argcount;
	xmlChar* arg_name;
	xmlChar* arg_data;

	for (be_dev = root->xmlChildrenNode; be_dev != NULL; be_dev = be_dev->next)
	{
		if(!xmlStrcmp(be_dev->name, (const xmlChar *)("BE_dev")))
		{
			// find BE_dev node
			break;
		}
	}

	// 
	if (be_dev == NULL)
		return -1;

	for (be_dev_data = be_dev->xmlChildrenNode; be_dev_data != NULL; be_dev_data = be_dev_data->next)
	{
		if(!xmlStrcmp(be_dev_data->name, (const xmlChar *)("data")))
		{
			if ((name = xmlGetProp(be_dev_data, "name")) == NULL)
				continue;
			
			if ((type = xmlGetProp(be_dev_data, "type")) == NULL)
				continue;

			if ((argcount = atoi(xmlGetProp(be_dev_data, "argcount"))) > AAL_BE_INIT_ARG_MAX_CNT)
				continue;

			//
			snprintf(conf->bed[conf->bedc].name, AAL_NAME_STRING_MAX_LEN, name);
			conf->bed[conf->bedc].beti = aal_be_type_name_to_index(conf, type);
			if (conf->bed[conf->bedc].beti == 0xff)
				continue;
						
			for (i = 0; i < argcount; i++)
			{
				uint8_t arg_prop_name[AAL_NAME_STRING_MAX_LEN];
				uint8_t arg_prop_data[AAL_ARG_STRING_MAX_LEN];
				snprintf(arg_prop_name, AAL_NAME_STRING_MAX_LEN, "arg%u_name", i+1);
				snprintf(arg_prop_data, AAL_NAME_STRING_MAX_LEN, "arg%u_data", i+1);
				if ((arg_name = xmlGetProp(be_dev_data, arg_prop_name)) == NULL)
					continue;
				
				if ((arg_data = xmlGetProp(be_dev_data, arg_prop_data)) == NULL)
					continue;
			
				snprintf(conf->bed[conf->bedc].beia.arg_name[conf->bed[conf->bedc].beia.num], AAL_NAME_STRING_MAX_LEN, arg_name);
				snprintf(conf->bed[conf->bedc].beia.arg[conf->bed[conf->bedc].beia.num], AAL_ARG_STRING_MAX_LEN, arg_data);
				conf->bed[conf->bedc].beia.num++;
			}

			if (argcount != conf->bed[conf->bedc].beia.num)
			{
				printf("backend dev '%s' arg error\n", name);
				continue;
			}

			conf->bedc++;

		}
	}

	if (conf->bedc == 0)
	{
		return -1;
	}

	return 0;
}


int aal_load_config_plugin_dev(struct aal_config* conf, xmlNodePtr root)
{
	uint32_t i;
	xmlNodePtr plugin_dev;
	xmlNodePtr plugin_dev_data;

	xmlChar* name;
	xmlChar* type;
	uint32_t argcount;
	xmlChar* arg_name;
	xmlChar* arg_data;

	for (plugin_dev = root->xmlChildrenNode; plugin_dev != NULL; plugin_dev = plugin_dev->next)
	{
		if(!xmlStrcmp(plugin_dev->name, (const xmlChar *)("plugin_dev")))
		{
			// find BE_dev node
			break;
		}
	}

	// 
	if (plugin_dev == NULL)
		return -1;

	for (plugin_dev_data = plugin_dev->xmlChildrenNode; plugin_dev_data != NULL; plugin_dev_data = plugin_dev_data->next)
	{
		if(!xmlStrcmp(plugin_dev_data->name, (const xmlChar *)("data")))
		{
			if ((name = xmlGetProp(plugin_dev_data, "name")) == NULL)
				continue;
			
			if ((type = xmlGetProp(plugin_dev_data, "type")) == NULL)
				continue;

			if ((argcount = atoi(xmlGetProp(plugin_dev_data, "argcount"))) > AAL_BE_INIT_ARG_MAX_CNT)
				continue;

			//
			snprintf(conf->pd[conf->pdc].name, AAL_NAME_STRING_MAX_LEN, name);
			conf->pd[conf->pdc].pti = aal_plugin_type_name_to_index(conf, type);
			if (conf->pd[conf->pdc].pti == 0xff)
				continue;
						
			for (i = 0; i < argcount; i++)
			{
				uint8_t arg_prop_name[AAL_NAME_STRING_MAX_LEN];
				uint8_t arg_prop_data[AAL_NAME_STRING_MAX_LEN];
				snprintf(arg_prop_name, AAL_NAME_STRING_MAX_LEN, "arg%u_name", i+1);
				snprintf(arg_prop_data, AAL_NAME_STRING_MAX_LEN, "arg%u_data", i+1);
				if ((arg_name = xmlGetProp(plugin_dev_data, arg_prop_name)) == NULL)
					continue;
				
				if ((arg_data = xmlGetProp(plugin_dev_data, arg_prop_data)) == NULL)
					continue;
			
				snprintf(conf->pd[conf->pdc].pia.arg_name[conf->pd[conf->pdc].pia.num], AAL_NAME_STRING_MAX_LEN, arg_name);
				snprintf(conf->pd[conf->pdc].pia.arg[conf->pd[conf->pdc].pia.num], AAL_ARG_STRING_MAX_LEN, arg_data);
				conf->pd[conf->pdc].pia.num++;
			}
			
			if (argcount != conf->pd[conf->pdc].pia.num)
			{
				printf("plugin dev '%s' arg error\n", name);
				continue;
			}

			conf->pdc++;

		}
	}
	
	if (conf->pdc == 0)
	{
		return -1;
	}

	return 0;
}

int aal_be_dev_name_to_index(struct aal_config* conf, char* name)
{
	uint32_t i;
	for (i = 0; i < conf->bedc; i++)
		if (strcmp(conf->bed[i].name, name) == 0)
			break;

	return i == conf->bedc ? 0xff : i;
}

int aal_plugin_dev_name_to_index(struct aal_config* conf, char* name)
{
	uint32_t i;
	for (i = 0; i < conf->pdc; i++)
		if (strcmp(conf->pd[i].name, name) == 0)
			break;

	return i == conf->pdc ? 0xff : i;
}



int aal_load_config_be_to_plugin(struct aal_config* conf, xmlNodePtr root)
{
	uint32_t i;
	xmlNodePtr be_to_plugin;
	xmlNodePtr be_to_plugin_data;

	xmlChar* be_dev;
	xmlChar* plugin_dev;
	uint32_t plugin_count;
	xmlChar* data_send_type;
	xmlChar* data_send_condition;
	xmlChar* xml_plugin_count;

	for (be_to_plugin = root->xmlChildrenNode; be_to_plugin != NULL; be_to_plugin = be_to_plugin->next)
	{
		if(!xmlStrcmp(be_to_plugin->name, (const xmlChar *)("BE_TO_plugin")))
		{
			// find BE_dev node
			break;
		}
	}

	// 
	if (be_to_plugin == NULL)
		return -1;

	for (be_to_plugin_data = be_to_plugin->xmlChildrenNode; be_to_plugin_data != NULL; be_to_plugin_data = be_to_plugin_data->next)
	{
		if(!xmlStrcmp(be_to_plugin_data->name, (const xmlChar *)("data")))
		{
			if ((be_dev = xmlGetProp(be_to_plugin_data, "BE_dev")) == NULL)
				continue;

			if ((xml_plugin_count = xmlGetProp(be_to_plugin_data, "plugin_count")) == NULL)
			{
				plugin_count = 1;
			}
			else
			{
				plugin_count = atoi(xml_plugin_count);
				if (plugin_count > AAL_BE_TO_PLUGIN_MAX_CNT)
					plugin_count = AAL_BE_TO_PLUGIN_MAX_CNT;
								
				if (plugin_count < 1)
					continue;
			}

			//
			conf->be2p[conf->be2pc].bedi = aal_be_dev_name_to_index(conf, be_dev);
			if (conf->be2p[conf->be2pc].bedi == 0xff)
				continue;

			if (plugin_count > 1)
			{
				if ((data_send_type = xmlGetProp(be_to_plugin_data, "data_send_type")) == NULL)
					continue;

				if (strcmp(data_send_type, "condition") == 0)
					conf->be2p[conf->be2pc].dst = 1; //condition
				else
					conf->be2p[conf->be2pc].dst = 0; //clone, or default(clone)
			}
			else
			{
				conf->be2p[conf->be2pc].dst = 0; //default clone
			}
						
			for (i = 0; i < plugin_count; i++)
			{
				uint8_t plugin_dev_name[AAL_NAME_STRING_MAX_LEN];
				snprintf(plugin_dev_name, AAL_NAME_STRING_MAX_LEN, "plugin_dev%u", i+1);
				if ((plugin_dev = xmlGetProp(be_to_plugin_data, plugin_dev_name)) == NULL)
					continue;
				
				conf->be2p[conf->be2pc].pdi[conf->be2p[conf->be2pc].pdic] = aal_plugin_dev_name_to_index(conf, plugin_dev);
				if (conf->be2p[conf->be2pc].pdi[conf->be2p[conf->be2pc].pdic] == 0xff)
					continue;

				if (conf->be2p[conf->be2pc].dst == 1)
				{
					uint8_t data_send_condition_name[AAL_NAME_STRING_MAX_LEN];
					snprintf(data_send_condition_name, AAL_NAME_STRING_MAX_LEN, "data_send_condition%u", i);
					if ((data_send_condition = xmlGetProp(be_to_plugin_data, data_send_condition_name)) == NULL)
						continue;

					snprintf(conf->be2p[conf->be2pc].cdr[conf->be2p[conf->be2pc].pdic], AAL_CONDITION_STRING_MAX_LEN, data_send_condition);
				}
				
				conf->be2p[conf->be2pc].pdic++;
			
			}
			
			if (plugin_count != conf->be2p[conf->be2pc].pdic)
			{
				printf("backend to plugin: plugin dev count error\n");
				continue;
			}

			conf->be2pc++;

		}
	}
	
	if (conf->be2pc == 0)
	{
		return -1;
	}

	return 0;
}


int aal_load_config_plugin_to_be(struct aal_config* conf, xmlNodePtr root)
{
	uint32_t i;
	xmlNodePtr plugin_to_be;
	xmlNodePtr plugin_to_be_data;

	xmlChar* be_dev;
	xmlChar* plugin_dev;
	uint32_t be_count;
	xmlChar* data_send_type;
	xmlChar* data_send_condition;
	xmlChar* xml_be_count;

	for (plugin_to_be = root->xmlChildrenNode; plugin_to_be != NULL; plugin_to_be = plugin_to_be->next)
	{
		if(!xmlStrcmp(plugin_to_be->name, (const xmlChar *)("plugin_TO_BE")))
		{
			// find BE_dev node
			break;
		}
	}

	// 
	if (plugin_to_be == NULL)
		return -1;

	for (plugin_to_be_data = plugin_to_be->xmlChildrenNode; plugin_to_be_data != NULL; plugin_to_be_data = plugin_to_be_data->next)
	{
		if(!xmlStrcmp(plugin_to_be_data->name, (const xmlChar *)("data")))
		{
			if ((plugin_dev = xmlGetProp(plugin_to_be_data, "plugin_dev")) == NULL)
				continue;
						
			if ((xml_be_count = xmlGetProp(plugin_to_be_data, "BE_count")) == NULL)
			{
				be_count = 1;
			}
			else
			{
				be_count = atoi(xml_be_count);
				if (be_count > AAL_PLUGIN_TO_BE_MAX_CNT)
					be_count = AAL_PLUGIN_TO_BE_MAX_CNT;
								
				if (be_count < 1)
					continue;
			}

			//
			conf->p2be[conf->p2bec].pdi = aal_plugin_dev_name_to_index(conf, plugin_dev);
			if (conf->p2be[conf->p2bec].pdi == 0xff)
				continue;

			if (be_count > 1)
			{
				if ((data_send_type = xmlGetProp(plugin_to_be_data, "data_send_type")) == NULL)
					continue;

				if (strcmp(data_send_type, "condition") == 0)
					conf->p2be[conf->p2bec].dst = 1; //condition
				else
					conf->p2be[conf->p2bec].dst = 0; //clone, or default(clone)
			}
			else
			{
				conf->p2be[conf->p2bec].dst = 0; //default clone
			}
						
			for (i = 0; i < be_count; i++)
			{
				uint8_t plugin_dev_name[AAL_NAME_STRING_MAX_LEN];
				snprintf(plugin_dev_name, AAL_NAME_STRING_MAX_LEN, "BE_dev%u", i+1);
				if ((be_dev = xmlGetProp(plugin_to_be_data, plugin_dev_name)) == NULL)
					continue;
				
				conf->p2be[conf->p2bec].bedi[conf->p2be[conf->p2bec].bedic] = aal_be_dev_name_to_index(conf, be_dev);
				if (conf->p2be[conf->p2bec].bedi[conf->p2be[conf->p2bec].bedic] == 0xff)
					continue;

				if (conf->p2be[conf->p2bec].dst == 1)
				{
					uint8_t data_send_condition_name[AAL_NAME_STRING_MAX_LEN];
					snprintf(data_send_condition_name, AAL_NAME_STRING_MAX_LEN, "data_send_condition%u", i);
					if ((data_send_condition = xmlGetProp(plugin_to_be_data, data_send_condition_name)) == NULL)
						continue;

					snprintf(conf->p2be[conf->p2bec].cdr[conf->p2be[conf->p2bec].bedic], AAL_CONDITION_STRING_MAX_LEN, data_send_condition);
				}
				
				conf->p2be[conf->p2bec].bedic++;
			
			}
			
			if (be_count != conf->p2be[conf->p2bec].bedic)
			{
				printf("plugin to backend: backend dev count error\n");
				continue;
			}

			conf->p2bec++;

		}
	}
	
	if (conf->p2bec == 0)
	{
		return -1;
	}

	return 0;
}

int aal_load_config(struct aal_config* conf)
{
	int ret = AAL_RET_OK;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	
	//
	if (conf == NULL)
		return AAL_RET_PARAMETER_ERR;

	//
	memset(conf, 0, sizeof(struct aal_config));
	
	if ((doc = xmlParseFile(AAL_CONFIG_XML_PATH)) == NULL) 
	{
		fprintf(stderr, "Failed to parser xml file:%s\n", AAL_CONFIG_XML_PATH);
		return -1;
	}

	if ((root_node = xmlDocGetRootElement(doc)) == NULL) 
	{
		fprintf(stderr, "Failed to get root node.\n");
		ret = -1;
		goto free_doc;
	}

	if (aal_load_config_be_type(conf, root_node) != 0)
	{
		fprintf(stderr, "Failed to load backend type.\n");
		ret = -1;
		goto free_doc;
	}

	
	if (aal_load_config_plugin_type(conf, root_node) != 0)
	{
		fprintf(stderr, "Failed to load plugin type.\n");
		ret = -1;
		goto free_doc;
	}
	
	if (aal_load_config_be_dev(conf, root_node) != 0)
	{
		fprintf(stderr, "Failed to load backend dev.\n");
		ret = -1;
		goto free_doc;
	}
	
	if (aal_load_config_plugin_dev(conf, root_node) != 0)
	{
		fprintf(stderr, "Failed to load plugin dev.\n");
		ret = -1;
		goto free_doc;
	}
	
	if (aal_load_config_be_to_plugin(conf, root_node) != 0)
	{
		fprintf(stderr, "Failed to load backend to plugin.\n");
		ret = -1;
		goto free_doc;
	}
	
	if (aal_load_config_plugin_to_be(conf, root_node) != 0)
	{
		fprintf(stderr, "Failed to load plugin to backend.\n");
		ret = -1;
		goto free_doc;
	}

free_doc:
	
	if (doc) 
		xmlFreeDoc(doc);

	return ret;
}


int aal_print_config(struct aal_config* conf)
{
	uint32_t i;
	uint32_t j;
	
	//
	if (conf == NULL)
		return AAL_RET_PARAMETER_ERR;

	for (i = 0; i < conf->betc; i++)
	{
		printf("backend type: name=%s, func(0x%p,0x%p,0x%p,0x%p)\n", 
			conf->bet[i].name, conf->bet[i].func.init, conf->bet[i].func.read, 
			conf->bet[i].func.write, conf->bet[i].func.clean);
	}

	printf("\n");
	
	for (i = 0; i < conf->ptc; i++)
	{
		printf("plugin type: name=%s, func(0x%p,0x%p,0x%p,0x%p)\n", 
			conf->pt[i].name, conf->pt[i].func.init, conf->pt[i].func.read, 
			conf->pt[i].func.write, conf->pt[i].func.clean);
	}

	printf("\n");

	for (i = 0; i < conf->bedc; i++)
	{
		printf("backend dev: name=%s, type index=%u, argcount=%u, ", 
			conf->bed[i].name, conf->bed[i].beti, conf->bed[i].beia.num);
		
		for (j = 0; j < conf->bed[i].beia.num; j++)
		{
			printf("arg%u: name=%s, data=%s; ", j,
				conf->bed[i].beia.arg_name[j], conf->bed[i].beia.arg[j]);
		}
		printf("\n");
	}

	printf("\n");

	
	for (i = 0; i < conf->pdc; i++)
	{
		printf("plugin dev: name=%s, type index=%u, argcount=%u, ", 
			conf->pd[i].name, conf->pd[i].pti, conf->pd[i].pia.num);
		
		for (j = 0; j < conf->pd[i].pia.num; j++)
		{
			printf("arg%u: name=%s, data=%s; ", j,  
				conf->pd[i].pia.arg_name[j], conf->pd[i].pia.arg[j]);
		}
		printf("\n");
	}

	printf("\n");


	
	for (i = 0; i < conf->be2pc; i++)
	{
		printf("backend to plugin: backend index=%u, plugin count=%u, data_send_type=%u, ", 
			conf->be2p[i].bedi, conf->be2p[i].pdic, conf->be2p[i].dst);
		
		for (j = 0; j < conf->be2p[i].pdic; j++)
		{
			printf("plugin%u: index=%u, ", j, conf->be2p[i].pdi[j]);
			
			if (conf->be2p[i].dst == 1)
				printf("condition=%s, ", conf->be2p[i].cdr[j]);
		}
		printf("\n");
	}

	printf("\n");

	
	for (i = 0; i < conf->p2bec; i++)
	{
		printf("plugin to backend: plugin index=%u, backend count=%u, data_send_type=%u, ", 
			conf->p2be[i].pdi, conf->p2be[i].bedic, conf->p2be[i].dst);
		
		for (j = 0; j < conf->p2be[i].bedic; j++)
		{
			printf("backend%u: index=%u, ", j, conf->p2be[i].bedi[j]);
			
			if (conf->p2be[i].dst == 1)
				printf("condition=%s, ", conf->p2be[i].cdr[j]);
		}
		printf("\n");
	}

	printf("\n");


	return 0;
}

int aal_unload_config(struct aal_config* conf)
{
	uint32_t i;

	for (i = 0; i < conf->betc; i++)
	{
		aal_unload_libbe(&conf->bet[i].func);
	}
	
	for (i = 0; i < conf->ptc; i++)
	{
		aal_unload_libplugin(&conf->pt[i].func);
	}
}

int aal_init_be_plugin(struct aal_config* conf)
{
	uint32_t i;
	for (i = 0; i < conf->bedc; i++)
		if (conf->bet[conf->bed[i].beti].func.init(&conf->bed[i].beird, &conf->bed[i].beia) != 0)
			goto init_be_err;

	for (i = 0; i < conf->pdc; i++)
		if (conf->pt[conf->pd[i].pti].func.init(&conf->pd[i].pird, &conf->pd[i].pia) != 0)
			goto init_plugin_err;
	
	return 0;

init_plugin_err:
	
	for (; i > 0; i++)
		conf->pt[conf->pd[i-1].pti].func.clean(&conf->pd[i-1].pird);

	i = conf->bedc;

init_be_err:

	for (; i > 0; i++)
		conf->bet[conf->bed[i-1].beti].func.clean(&conf->bed[i-1].beird);

	return -1;
}

int aal_clean_be_plugin(struct aal_config* conf)
{
	uint32_t i;
	for (i = 0; i < conf->bedc; i++)
		conf->bet[conf->bed[i].beti].func.clean(conf->bed[i].beird);

	for (i = 0; i < conf->pdc; i++)
		conf->pt[conf->bed[i].beti].func.clean(conf->pd[i].pird);

	return 0;
}


void* aal_be2plugin(void* indata)
{
	uint32_t i;
	struct aal_be2p_thread_arg* arg = (struct aal_be2p_thread_arg*) indata;

	struct be2plugin* be2plugin = &arg->conf->be2p[arg->be2pi];
	struct be_dev* be_dev = &arg->conf->bed[be2plugin->bedi];
	struct be_type* be_type = &arg->conf->bet[be_dev->beti];

	struct plugin_dev* plugin_dev;
	struct plugin_type* plugin_type;
	
	int (*be_read)(void*, void**) = be_type->func.read;

	void* data;
	int readlen;
	int writelen;
	while(1)
	{
		readlen = be_read((void*) be_dev->beird, &data);
		if (readlen <= 0)
			continue;

		be_dev->statistics.read_num ++;
		be_dev->statistics.read_bytes += readlen;

		if (be2plugin->dst == 1)
		{
			for (i = 0; i < be2plugin->pdic; i++)
			{
				if (/*reg match*/1)
				{
					plugin_dev = &arg->conf->pd[be2plugin->pdi[i]];
					plugin_type = &arg->conf->pt[plugin_dev->pti];

					writelen = plugin_type->func.write(plugin_dev->pird, data, readlen);
					if (writelen != readlen)
					{
						printf("be2plugin error\n");
						continue;
					}
					
					plugin_dev->statistics.write_num ++;
					plugin_dev->statistics.write_bytes += readlen;
				}
			}
		}
		else // dstt == 0 or default
		{
			for (i = 0; i < be2plugin->pdic; i++)
			{
				plugin_dev = &arg->conf->pd[be2plugin->pdi[i]];
				plugin_type = &arg->conf->pt[plugin_dev->pti];
				
				writelen = plugin_type->func.write(plugin_dev->pird, data, readlen);
				if (writelen != readlen)
				{
					printf("be2plugin error\n");
					continue;
				}
				
				plugin_dev->statistics.write_num ++;
				plugin_dev->statistics.write_bytes += readlen;
			}
		}
	}
	return NULL;
}

void* aal_plugin2be(void* indata)
{
	uint32_t i;
	struct aal_p2be_thread_arg* arg = (struct aal_p2be_thread_arg*) indata;

	struct plugin2be* plugin2be = &arg->conf->p2be[arg->p2bei];
	struct plugin_dev* plugin_dev = &arg->conf->pd[plugin2be->pdi];
	struct plugin_type* plugin_type = &arg->conf->pt[plugin_dev->pti];
	
	struct be_dev* be_dev;
	struct be_type* be_type;

	void* data;
	int readlen;
	int writelen;
	while(1)
	{
		readlen = plugin_type->func.read(plugin_dev->pird, &data);
		if (readlen <= 0)
			continue;
		
		plugin_dev->statistics.read_num ++;
		plugin_dev->statistics.read_bytes += readlen;

		if (plugin2be->dst == 1)
		{
			for (i = 0; i < plugin2be->bedic; i++)
			{
				if (/*reg match*/1)
				{
					be_dev = &arg->conf->bed[plugin2be->bedi[i]];
					be_type = &arg->conf->bet[be_dev->beti];
					
					writelen = be_type->func.write(be_dev->beird, data, readlen);
					if (writelen != readlen)
					{
						printf("plugin2be error\n");
						continue;
					}
					
					be_dev->statistics.write_num ++;
					be_dev->statistics.write_bytes += readlen;
				}
			}
			
		}
		else // dstt == 0 or default
		{
			for (i = 0; i < plugin2be->bedic; i++)
			{
				be_dev = &arg->conf->bed[plugin2be->bedi[i]];
				be_type = &arg->conf->bet[be_dev->beti];
				
				writelen = be_type->func.write(be_dev->beird, data, readlen);
				if (writelen != readlen)
				{
					printf("plugin2be error\n");
					continue;
				}
				
				be_dev->statistics.write_num ++;
				be_dev->statistics.write_bytes += readlen;
			}
		}
	}

	return NULL;
}


void aal_print_statistics(struct aal_config* conf)
{
	uint32_t i;

	printf("\n\n\n\n\n");
	printf("backend: \n");
	for (i = 0; i < conf->bedc; i++)
	{
		printf("%32s: Read : %10lu times   - %10lu Bytes,   Write: %10lu times   - %10lu Bytes\n", 
			conf->bed[i].name, 
			conf->bed[i].statistics.read_num, conf->bed[i].statistics.read_bytes, 
			conf->bed[i].statistics.write_num, conf->bed[i].statistics.write_bytes);
		
		printf("%32s: Read : %10lu times/s - %10lu Bytes/s, Write: %10lu times/s - %10lu Bytes/s\n", 
			"", 
			conf->bed[i].statistics.read_num - conf->bed[i].last_second.read_num, 
			conf->bed[i].statistics.read_bytes - conf->bed[i].last_second.read_bytes, 
			conf->bed[i].statistics.write_num - conf->bed[i].last_second.write_num, 
			conf->bed[i].statistics.write_bytes - conf->bed[i].last_second.write_bytes);

		memcpy(&conf->bed[i].last_second, &conf->bed[i].statistics, sizeof(struct be_statistics));
	}
	printf("\n");
	printf("plugin: \n");
	for (i = 0; i < conf->pdc; i++)
	{
		printf("%32s: Write: %10lu times   - %10lu Bytes,   Read : %10lu times   - %10lu Bytes\n", 
			conf->pd[i].name, conf->pd[i].statistics.write_num, conf->pd[i].statistics.write_bytes, 
			conf->pd[i].statistics.read_num, conf->pd[i].statistics.read_bytes);
		
		printf("%32s: Write: %10lu times/s - %10lu Bytes/s, Read : %10lu times/s - %10lu Bytes/s\n", 
			"", 
			conf->pd[i].statistics.write_num - conf->pd[i].last_second.write_num, 
			conf->pd[i].statistics.write_bytes - conf->pd[i].last_second.write_bytes, 
			conf->pd[i].statistics.read_num - conf->pd[i].last_second.read_num, 
			conf->pd[i].statistics.read_bytes - conf->pd[i].last_second.read_bytes);
		
		memcpy(&conf->pd[i].last_second, &conf->pd[i].statistics, sizeof(struct plugin_statistics));
	}

	printf("\n");

}

/*******************/

int aal_init(struct aal_config* conf)
{
	if (aal_load_config(conf) != 0)
	{
		printf("AAL config error.\n");
		return -1;
	}
	
	aal_print_config(conf);

	if (aal_init_be_plugin(conf) != 0)
	{
		printf("AAL init backend plugin error.\n");
		return -1;
	}

	return 0;
}

int aal_run(struct aal_config* conf)
{
	uint32_t i;
	pthread_t pid;
	struct aal_be2p_thread_arg be2p_arg[AAL_BE_TO_PLUGIN_MAX_NUM];
	struct aal_p2be_thread_arg p2be_arg[AAL_PLUGIN_TO_BE_MAX_NUM];
	
	for (i = 0; i < conf->be2pc; i++)
	{
		be2p_arg[i].be2pi = i;
		be2p_arg[i].conf = conf;

		pthread_create(&pid, NULL, aal_be2plugin, &be2p_arg[i]);
	}

	for (i = 0; i < conf->p2bec; i++)
	{
		p2be_arg[i].p2bei = i;
		p2be_arg[i].conf = conf;
		
		pthread_create(&pid, NULL, aal_plugin2be, &p2be_arg[i]);
	}
	
	while(1)
	{
	#if 0
		sleep(3600);
	#else
		aal_print_statistics(conf);
		sleep(1);
	#endif
	}

	return 0;
}

int aal_clean(struct aal_config* conf)
{
	aal_unload_config(conf);
	aal_clean_be_plugin(conf);
	return 0;
}

