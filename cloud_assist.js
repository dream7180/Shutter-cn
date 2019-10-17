// ==UserScript==
// @name              百度网盘下载助理
// @namespace         https://github.com/syhyz1990/baiduyun
// @version           1.0.0
// @icon              https://www.baidu.com/favicon.ico
// @description       精简修改自【百度网盘直链下载助手】2.8.5-https://github.com/syhyz1990/baiduyun
// @author            syhyz1990,dreamawake
// @license           MIT
// @supportURL        https://github.com/syhyz1990/baiduyun
// @match             *://pan.baidu.com/disk/home*
// @match             *://yun.baidu.com/disk/home*
// @match             *://pan.baidu.com/s/*
// @match             *://yun.baidu.com/s/*
// @match             *://pan.baidu.com/share/*
// @match             *://yun.baidu.com/share/*
// @require           https://cdn.bootcss.com/jquery/1.12.4/jquery.min.js
// @require           https://cdn.bootcss.com/sweetalert/2.1.2/sweetalert.min.js
// @connect           baidu.com
// @connect           meek.com.cn
// @run-at            document-idle
// @grant             unsafeWindow
// @grant             GM_xmlhttpRequest
// @grant             GM_setClipboard
// @grant             GM_setValue
// @grant             GM_getValue
// @grant             GM_deleteValue
// @grant             GM_openInTab
// @grant             GM_registerMenuCommand
// @grant             GM_unregisterMenuCommand
// ==/UserScript==

'use strict'

;(function () {
  const version = '1.0.0';
  const classMap = {
    'list': 'zJMtAEb',
    'grid': 'fyQgAEb',
    'list-grid-switch': 'auiaQNyn',
    'list-switched-on': 'ewXm1e',
    'grid-switched-on': 'kxhkX2Em',
    'list-switch': 'rvpXm63',
    'grid-switch': 'mxgdJgwv',
    'checkbox': 'EOGexf',
    'col-item': 'Qxyfvg',
    'check': 'fydGNC',
    'checked': 'EzubGg',
    'chekbox-grid': 'cEefyz',
    'list-view': 'vdAfKMb',
    'item-active': 'ihpXRy',
    'grid-view': 'JKvHJMb',
    'bar-search': 'OFaPaO',
    'list-tools': 'tcuLAu',
    'header': 'vyQHNyb'
  };
  const errorMsg = {
    'dir': '提示：此方式不支持整个文件夹下载，可进入文件夹内获取文件链接下载',
    'unlogin': '提示：必须登录百度网盘后才能使用此功能哦!!!',
    'fail': '提示：获取下载链接失败！请刷新网页后重试！',
    'unselected': '提示：未选中文件，请勿勾选文件夹，否则刷新后重试！',
    'morethan': '提示：多个文件请点击【显示链接】',
    'toobig': '提示：只支持300M以下的文件夹，若链接无法下载，请进入文件夹后勾选文件获取！'
  };

  const secretCode = GM_getValue('secretCode') ? GM_getValue('secretCode') : '752149';
  const savePath = GM_getValue('savePath') ? GM_getValue('savePath') : '/我的资源';
  const userAgent = "netdisk;9.6.23;MI 8;9;JSbridge4.0.0";

  function clog(c1, c2, c3) {
    c1 = c1 ? c1 : '';
    c2 = c2 ? c2 : '';
    c3 = c3 ? c3 : '';
    console.group('[百度网盘下载助理]');
    console.log(c1, c2, c3);
    console.groupEnd();
  }

  //网盘页面的下载助手
  function PanHelper() {
    let yunData, sign, timestamp, bdstoken, logid, fid_list;
    let fileList = [], selectFileList = [], batchLinkList = [], batchLinkListAll = [], linkList = [],
        list_grid_status = 'list';
    let observer, currentPage, currentPath, currentCategory, dialog, searchKey;
    let panAPIUrl = location.protocol + "//" + location.host + "/api/";
    let restAPIUrl = location.protocol + "//pcs.baidu.com/rest/2.0/pcs/";
    let clientAPIUrl = location.protocol + "//d.pcs.baidu.com/rest/2.0/pcs/";

    this.init = function () {
      yunData = unsafeWindow.yunData;
      clog('初始化信息:', yunData);
      if (yunData === undefined) {
        clog('页面未正常加载，或者百度已经更新！');
        return;
      }
      initParams();
      registerEventListener();
      createObserver();
      addButton();
      createIframe();
      dialog = new Dialog({addCopy: true});
      clog('下载助理加载成功！当前版本：', version);
    };

    function initParams() {
      sign = getSign();
      timestamp = getTimestamp();
      bdstoken = getBDStoken();
      logid = getLogID();
      currentPage = getCurrentPage();

      if (currentPage == 'all')
        currentPath = getPath();
      if (currentPage == 'category')
        currentCategory = getCategory();
      if (currentPage == 'search')
        searchKey = getSearchKey();
      refreshListGridStatus();
      refreshFileList();
      refreshSelectList();
    }

    function refreshFileList() {
      if (currentPage == 'all') {
        fileList = getFileList();
      } else if (currentPage == 'category') {
        fileList = getCategoryFileList();
      } else if (currentPage == 'search') {
        fileList = getSearchFileList();
      }
    }

    function refreshSelectList() {
      selectFileList = [];
    }

    function refreshListGridStatus() {
      list_grid_status = getListGridStatus();
    }

    //获取当前的视图模式
    function getListGridStatus() {
      if ($('.' + classMap['list']).is(':hidden')) {
        return 'grid';
      } else {
        return 'list';
      }
    }

    function registerEventListener() {
      registerHashChange();
      registerListGridStatus();
      registerCheckbox();
      registerAllCheckbox();
      registerFileSelect();
      registerShareClick();
    }

    //监视点击分享按钮
    function registerShareClick() {
      $(document).on('click', '[title="分享"]', function () {
        let inv = setInterval(function () {
          if ($('#share-method-public').length === 0) {
            $(".share-method-line").parent().append('<div class="share-method-line"><input type="radio" id="share-method-public" name="share-method" value="public" checked><span class="icon radio-icon icon-radio-non"></span><label for="share-method-public"><b>公开分享</b><span>任何人访问链接即可查看，下载！</span></div>');
          } else {
            clearInterval(inv);
            $(document).off('click', '[title="分享"]');
          }
        }, 100);
      });
    }

    //监视地址栏#标签的变化
    function registerHashChange() {
      window.addEventListener('hashchange', function (e) {
        refreshListGridStatus();

        if (getCurrentPage() == 'all') {
          if (currentPage == getCurrentPage()) {
            if (currentPath != getPath()) {
              currentPath = getPath();
              refreshFileList();
              refreshSelectList();
            }
          } else {
            currentPage = getCurrentPage();
            currentPath = getPath();
            refreshFileList();
            refreshSelectList();
          }
        } else if (getCurrentPage() == 'category') {
          if (currentPage == getCurrentPage()) {
            if (currentCategory != getCategory()) {
              currentPage = getCurrentPage();
              currentCategory = getCategory();
              refreshFileList();
              refreshSelectList();
            }
          } else {
            currentPage = getCurrentPage();
            currentCategory = getCategory();
            refreshFileList();
            refreshSelectList();
          }
        } else if (getCurrentPage() == 'search') {
          if (currentPage == getCurrentPage()) {
            if (searchKey != getSearchKey()) {
              currentPage = getCurrentPage();
              searchKey = getSearchKey();
              refreshFileList();
              refreshSelectList();
            }
          } else {
            currentPage = getCurrentPage();
            searchKey = getSearchKey();
            refreshFileList();
            refreshSelectList();
          }
        }
      });
    }

    //监视视图变化
    function registerListGridStatus() {
      let $a_list = $('a[data-type=list]');
      $a_list.click(function () {
        list_grid_status = 'list';
      });

      let $a_grid = $('a[data-type=grid]');
      $a_grid.click(function () {
        list_grid_status = 'grid';
      });
    }

    //文件选择框
    function registerCheckbox() {
      let $checkbox = $('span.' + classMap['checkbox']);
      if (list_grid_status == 'grid') {
        $checkbox = $('.' + classMap['chekbox-grid']);
      }

      $checkbox.each(function (index, element) {
        $(element).on('click', function (e) {
          let $parent = $(this).parent(), filename, isActive;

          if (list_grid_status == 'list') {
            filename = $('div.file-name div.text a', $parent).attr('title');
            isActive = $parent.hasClass(classMap['item-active']);
          } else if (list_grid_status == 'grid') {
            filename = $('div.file-name a', $(this)).attr('title');
            isActive = !$(this).hasClass(classMap['item-active']);
          }

          if (isActive) {
            clog('取消选中文件：' + filename);
            for (let i = 0; i < selectFileList.length; i++) {
              if (selectFileList[i].filename == filename) {
                selectFileList.splice(i, 1);
              }
            }
          } else {
            clog('选中文件:' + filename);
            $.each(fileList, function (index, element) {
              if (element.server_filename == filename) {
                let obj = {
                  filename: element.server_filename,
                  path: element.path,
                  fs_id: element.fs_id,
                  isdir: element.isdir
                };
                selectFileList.push(obj);
              }
            });
          }
        });
      });
    }

    function unregisterCheckbox() {
      let $checkbox = $('span.' + classMap['checkbox']);
      $checkbox.each(function (index, element) {
        $(element).unbind('click');
      });
    }

    //全选框
    function registerAllCheckbox() {
      let $checkbox = $('div.' + classMap['col-item'] + '.' + classMap['check']);
      $checkbox.each(function (index, element) {
        $(element).bind('click', function (e) {
          let $parent = $(this).parent();
          if ($parent.hasClass(classMap['checked'])) {
            clog('取消全选');
            selectFileList = [];
          } else {
            clog('全部选中');
            selectFileList = [];
            $.each(fileList, function (index, element) {
              let obj = {
                filename: element.server_filename,
                path: element.path,
                fs_id: element.fs_id,
                isdir: element.isdir
              };
              selectFileList.push(obj);
            });
          }
        });
      });
    }

    function unregisterAllCheckbox() {
      let $checkbox = $('div.' + classMap['col-item'] + '.' + classMap['check']);
      $checkbox.each(function (index, element) {
        $(element).unbind('click');
      });
    }

    //单个文件选中，点击文件不是点击选中框，会只选中该文件
    function registerFileSelect() {
      let $dd = $('div.' + classMap['list-view'] + ' dd');
      $dd.each(function (index, element) {
        $(element).bind('click', function (e) {
          let nodeName = e.target.nodeName.toLowerCase();
          if (nodeName != 'span' && nodeName != 'a' && nodeName != 'em') {
            clog('shiftKey:' + e.shiftKey);
            if (!e.shiftKey) {
              selectFileList = [];
              let filename = $('div.file-name div.text a', $(this)).attr('title');
              clog('选中文件：' + filename);
              $.each(fileList, function (index, element) {
                if (element.server_filename == filename) {
                  let obj = {
                    filename: element.server_filename,
                    path: element.path,
                    fs_id: element.fs_id,
                    isdir: element.isdir
                  };
                  selectFileList.push(obj);
                }
              });
            } else {
              selectFileList = [];
              let $dd_select = $('div.' + classMap['list-view'] + ' dd.' + classMap['item-active']);
              $.each($dd_select, function (index, element) {
                let filename = $('div.file-name div.text a', $(element)).attr('title');
                clog('选中文件：' + filename);
                $.each(fileList, function (index, element) {
                  if (element.server_filename == filename) {
                    let obj = {
                      filename: element.server_filename,
                      path: element.path,
                      fs_id: element.fs_id,
                      isdir: element.isdir
                    };
                    selectFileList.push(obj);
                  }
                });
              });
            }
          }
        });
      });
    }

    function unregisterFileSelect() {
      let $dd = $('div.' + classMap['list-view'] + ' dd');
      $dd.each(function (index, element) {
        $(element).unbind('click');
      });
    }

    //监视文件列表显示变化
    function createObserver() {
      let MutationObserver = window.MutationObserver;
      let options = {
        'childList': true
      };
      observer = new MutationObserver(function (mutations) {
        unregisterCheckbox();
        unregisterAllCheckbox();
        unregisterFileSelect();
        registerCheckbox();
        registerAllCheckbox();
        registerFileSelect();
      });

      let list_view = document.querySelector('.' + classMap['list-view']);
      let grid_view = document.querySelector('.' + classMap['grid-view']);

      observer.observe(list_view, options);
      observer.observe(grid_view, options);
    }

    //我的网盘 - 添加助手按钮
    function addButton() {
      $('div.' + classMap['bar-search']).css('width', '18%');
      let $dropdownbutton = $('<span class="g-dropdown-button"></span>');
      let $dropdownbutton_a = $('<a class="g-button g-button-blue" href="javascript:;"><span class="g-button-right"><em class="icon icon-speed" title="百度网盘下载助理"></em><span class="text" style="width: 60px;">下载助理</span></span></a>');
      let $dropdownbutton_span = $('<span class="menu" style="width:114px"></span>');

      let $directbutton_batchhttplink_button = $('<a id="batchhttplink-direct" class="g-button-menu" href="javascript:;">显示直链</a>');
      $directbutton_batchhttplink_button.click(batchClick);
      let $apibutton_download_button = $('<a id="download-api" class="g-button-menu" href="javascript:;">API下载</a>');
      let $apibutton_batchhttplink_button = $('<a id="batchhttplink-api" class="g-button-menu" href="javascript:;">显示API链接</a>');
      $apibutton_download_button.click(downloadClick);
      $apibutton_batchhttplink_button.click(batchClick);
      let $sharebutton = $('<span class="g-button-menu" style="display:block;cursor: pointer">免密分享</span>');
      $dropdownbutton_span.append($directbutton_batchhttplink_button).append($apibutton_download_button).append($apibutton_batchhttplink_button).append($sharebutton);
      $dropdownbutton.append($dropdownbutton_a).append($dropdownbutton_span);

      $dropdownbutton.hover(function () {
        $dropdownbutton.toggleClass('button-open');
      });

      $sharebutton.click(getLinkWithShare);

      $('.' + classMap['list-tools']).append($dropdownbutton);
      $('.' + classMap['list-tools']).css('height', '40px');
    }

    // 我的网盘 - 下载
    function downloadClick(event) {
      clog('选中文件列表：', selectFileList);
      let id = event.target.id;
      let downloadLink;

      if (id == 'download-direct') {
        let downloadType;
        if (selectFileList.length === 0) {
          swal(errorMsg.unselected);
          return;
        }
        if (selectFileList.length == 1) {
          selectFileList[0].isdir === 1 ? downloadType = 'batch' : downloadType = 'dlink';
        }
        if (selectFileList.length > 1) {
          downloadType = 'batch';
        }

        fid_list = getFidList(selectFileList);
        let result = getDownloadLinkWithPanAPI(downloadType);
        if (result.errno === 0) {
          if (downloadType == 'dlink')
            downloadLink = result.dlink[0].dlink;
          else if (downloadType == 'batch') {
            downloadLink = result.dlink;
            if (selectFileList.length === 1)
              downloadLink = downloadLink + '&zipname=' + encodeURIComponent(selectFileList[0].filename) + '.zip';
          } else {
            swal("发生错误！");
            return;
          }
        } else if (result.errno == -1) {
          swal('文件不存在或已被百度和谐，无法下载！');
          return;
        } else if (result.errno == 112) {
          swal("页面过期，请刷新重试！");
          return;
        } else {
          swal("发生错误！");
          return;
        }
      } else {
        if (selectFileList.length === 0) {
          swal(errorMsg.unselected);
          return;
        } else if (selectFileList.length > 1) {
          swal(errorMsg.morethan);
          return;
        } else {
          if (selectFileList[0].isdir == 1) {
            swal(errorMsg.dir);
            return;
          }
        }
        if (id == 'download-api') {
          downloadLink = getDownloadLinkWithRESTAPIBaidu(selectFileList[0].path);
        }
      }
      execDownload(downloadLink);
    }

    //我的网盘 - 显示链接
    function linkClick(event) {
      clog('选中文件列表：', selectFileList);
      let id = event.target.id;
      let linkList, tip;

      if (id.indexOf('direct') != -1) {
        let downloadType;
        let downloadLink;
        if (selectFileList.length === 0) {
          swal(errorMsg.unselected);
          return;
        } else if (selectFileList.length == 1) {
          if (selectFileList[0].isdir === 1)
            downloadType = 'batch';
          else if (selectFileList[0].isdir === 0)
            downloadType = 'dlink';
        } else if (selectFileList.length > 1) {
          downloadType = 'batch';
        }
        fid_list = getFidList(selectFileList);
        let result = getDownloadLinkWithPanAPI(downloadType);
        if (result.errno === 0) {
          if (downloadType == 'dlink')
            downloadLink = result.dlink[0].dlink;
          else if (downloadType == 'batch') {
            clog('选中文件列表：', selectFileList);
            downloadLink = result.dlink;
            if (selectFileList.length === 1)
              downloadLink = downloadLink + '&zipname=' + encodeURIComponent(selectFileList[0].filename) + '.zip';
          } else {
            swal("发生错误！");
            return;
          }
        } else if (result.errno == -1) {
          swal('文件不存在或已被百度和谐，无法下载！');
          return;
        } else if (result.errno == 112) {
          swal("页面过期，请刷新重试！");
          return;
        } else {
          swal("发生错误！");
          return;
        }
        let httplink = downloadLink.replace(/^([A-Za-z]+):/, 'http:');
        let httpslink = downloadLink.replace(/^([A-Za-z]+):/, 'https:');
        let filename = '';
        $.each(selectFileList, function (index, element) {
          if (selectFileList.length == 1)
            filename = element.filename;
          else {
            if (index == 0)
              filename = element.filename;
            else
              filename = filename + ',' + element.filename;
          }
        });
        linkList = {
          filename: filename,
          urls: [
            {url: httplink, rank: 1},
            {url: httpslink, rank: 2}
          ]
        };
        tip = '显示模拟百度网盘网页获取的链接，可以使用右键迅雷或IDM下载，多文件打包(限300k)下载的链接可以直接复制使用';
        dialog.open({title: '下载链接', type: 'link', list: linkList, tip: tip});
      } else {
        if (selectFileList.length === 0) {
          swal(errorMsg.unselected);
          return;
        } else if (selectFileList.length > 1) {
          swal(errorMsg.morethan);
          return;
        } else {
          if (selectFileList[0].isdir == 1) {
            swal(errorMsg.dir);
            return;
          }
        }
        if (id.indexOf('api') != -1) {
          let downloadLink = getDownloadLinkWithRESTAPIBaidu(selectFileList[0].path);
          let httplink = downloadLink.replace(/^([A-Za-z]+):/, 'http:');
          let httpslink = downloadLink.replace(/^([A-Za-z]+):/, 'https:');
          linkList = {
            filename: selectFileList[0].filename,
            urls: [
              {url: httplink, rank: 1},
              {url: httpslink, rank: 2}
            ]
          };

          tip = '显示模拟APP获取的链接(使用百度云ID)，可以右键使用迅雷或IDM下载，直接复制链接无效';
          dialog.open({title: '下载链接', type: 'link', list: linkList, tip: tip});
        }
      }
    }

    // 我的网盘 - 批量下载
    function batchClick(event) {
      clog('选中文件列表：', selectFileList);
      if (selectFileList.length === 0) {
        swal(errorMsg.unselected);
        return;
      }
      let id = event.target.id;
      let linkType, tip;
      linkType = id.indexOf('https') == -1 ? (id.indexOf('http') == -1 ? location.protocol + ':' : 'http:') : 'https:';
      batchLinkList = [];
      batchLinkListAll = [];
      if (id.indexOf('direct') != -1) {  //直链下载
        batchLinkList = getDirectBatchLink(linkType);
        let tip = '支持使用IDM批量下载，需升级 <a href="https://www.baiduyun.wiki/zh-cn/assistant.html">[百度网盘万能助手]</a> 至v2.0.1';
        if (batchLinkList.length === 0) {
          swal('没有链接可以显示，不要选中文件夹！');
          return;
        }
        dialog.open({title: '直链下载', type: 'batch', list: batchLinkList, tip: tip, showcopy: true});
      }
	  if (id.indexOf('api') != -1) {
        batchLinkList = getAPIBatchLink(linkType);
        tip = '请先安装 <a href="https://www.baiduyun.wiki/zh-cn/assistant.html">百度网盘万能助手</a> 请将链接复制到“IDM->任务->从剪切板中添加批量下载”';
        if (batchLinkList.length === 0) {
          swal('没有链接可以显示，API链接不要全部选中文件夹！');
          return;
        }
        dialog.open({title: 'API下载链接', type: 'batch', list: batchLinkList, tip: tip,showcopy: true});
      }
    }

    //我的网盘 - 获取直链下载地址
    function getDirectBatchLink(linkType) {
      let list = [];
      $.each(selectFileList, function (index, element) {
        let downloadType, downloadLink, result;
        if (element.isdir == 0)
          downloadType = 'dlink';
        else
          downloadType = 'batch';
        fid_list = getFidList([element]);
        result = getDownloadLinkWithPanAPI(downloadType);
        if (result.errno == 0) {
          if (downloadType == 'dlink')
            downloadLink = result.dlink[0].dlink;
          else if (downloadType == 'batch')
            downloadLink = result.dlink;
          downloadLink = downloadLink.replace(/^([A-Za-z]+):/, linkType);
        } else {
          downloadLink = 'error';
        }
        list.push({filename: element.filename, downloadlink: downloadLink});
      });
      return list;
    }

    //我的网盘 - 获取API下载地址
    function getAPIBatchLink(linkType) {
      let list = [];
      $.each(selectFileList, function (index, element) {
        if (element.isdir == 1)
          return;
        let downloadLink;
        downloadLink = getDownloadLinkWithRESTAPIBaidu(element.path);
        downloadLink = downloadLink.replace(/^([A-Za-z]+):/, linkType);
        list.push({filename: element.filename, downloadlink: downloadLink});
      });
      return list;
    }

    function getSign() {
      let signFnc;
      try {
        signFnc = new Function("return " + yunData.sign2)();
      } catch (e) {
        throw new Error(e.message);
      }
      return base64Encode(signFnc(yunData.sign5, yunData.sign1));
    }

    //获取当前目录
    function getPath() {
      let hash = location.hash;
      let regx = new RegExp("path=([^&]*)(&|$)", 'i');
      let result = hash.match(regx);
      return decodeURIComponent(result[1]);
    }

    //获取分类显示的类别，即地址栏中的type
    function getCategory() {
      let hash = location.hash;
      let regx = new RegExp("type=([^&]*)(&|$)", 'i');
      let result = hash.match(regx);
      return decodeURIComponent(result[1]);
    }

    function getSearchKey() {
      let hash = location.hash;
      let regx = new RegExp("key=([^&]*)(&|$)", 'i');
      let result = hash.match(regx);
      return decodeURIComponent(result[1]);
    }

    //获取当前页面(all或者category或search)
    function getCurrentPage() {
      let hash = location.hash;
      return hash.substring(hash.indexOf('#') + 2, hash.indexOf('?'));
    }

    //获取文件列表
    function getFileList() {
      let filelist = [];
      let listUrl = panAPIUrl + "list";
      let path = getPath();
      logid = getLogID();
      let params = {
        dir: path,
        bdstoken: bdstoken,
        logid: logid,
        order: 'size',
        num: 1000,
        desc: 0,
        clienttype: 0,
        showempty: 0,
        web: 1,
        channel: 'chunlei',
        appid: secretCode
      };

      $.ajax({
        url: listUrl,
        async: false,
        method: 'GET',
        data: params,
        success: function (response) {
          filelist = 0 === response.errno ? response.list : [];
        }
      });
      return filelist;
    }

    //获取分类页面下的文件列表
    function getCategoryFileList() {
      let filelist = [];
      let listUrl = panAPIUrl + "categorylist";
      let category = getCategory();
      logid = getLogID();
      let params = {
        category: category,
        bdstoken: bdstoken,
        logid: logid,
        order: 'size',
        desc: 0,
        clienttype: 0,
        showempty: 0,
        web: 1,
        channel: 'chunlei',
        appid: secretCode
      };
      $.ajax({
        url: listUrl,
        async: false,
        method: 'GET',
        data: params,
        success: function (response) {
          filelist = 0 === response.errno ? response.info : [];
        }
      });
      return filelist;
    }

    function getSearchFileList() {
      let filelist = [];
      let listUrl = panAPIUrl + 'search';
      logid = getLogID();
      searchKey = getSearchKey();
      let params = {
        recursion: 1,
        order: 'time',
        desc: 1,
        showempty: 0,
        web: 1,
        page: 1,
        num: 100,
        key: searchKey,
        channel: 'chunlei',
        app_id: 250528,
        bdstoken: bdstoken,
        logid: logid,
        clienttype: 0
      };
      $.ajax({
        url: listUrl,
        async: false,
        method: 'GET',
        data: params,
        success: function (response) {
          filelist = 0 === response.errno ? response.list : [];
        }
      });
      return filelist;
    }

    //生成下载时的fid_list参数
    function getFidList(list) {
      let fidlist = null;
      if (list.length === 0)
        return null;
      let fileidlist = [];
      $.each(list, function (index, element) {
        fileidlist.push(element.fs_id);
      });
      fidlist = '[' + fileidlist + ']';
      return fidlist;
    }

    function getTimestamp() {
      return yunData.timestamp;
    }

    function getBDStoken() {
      return yunData.MYBDSTOKEN;
    }

    function getLinkWithShare() {
      let path = [];
      if (selectFileList.length === 0) {
        swal(errorMsg.unselected);
        return;
      }

      $.each(selectFileList, function (i, val) {
        path.push(val['path']);
      });

      let shareAPIUrl = "https://pan.baidu.com/share/pset?channel=chunlei&clienttype=0&web=1&channel=chunlei&web=1&app_id=250528&bdstoken=" + bdstoken + "&logid=" + logid + "&clienttype=0";

      let params = {
        schannel: 0,
        channel_list: JSON.stringify([]),
        period: 7,
        path_list: JSON.stringify(path)
      };

      $.ajax({
        url: shareAPIUrl,
        async: false,
        method: 'POST',
        data: params,
        success: function (res) {
          if (res.errno === 0) {
            swal({
              title: "分享链接",
              text: res.link,
              buttons: {confirm: {text: "打开", value: 'confirm'}}
            }).then((value) => {
              if (value === 'confirm') {
                GM_openInTab(res.link, {active: true});
              }
            });
          }
        }
      });
    }

    //获取直接下载地址
    //这个地址不是直接下载地址，访问这个地址会返回302，response header中的location才是真实下载地址
    //暂时没有找到提取方法
    function getDownloadLinkWithPanAPI(type) {
      let result;
      logid = getLogID();
      let query = {
        bdstoken: bdstoken,
        logid: logid,
      };
      let params = {
        sign: sign,
        timestamp: timestamp,
        fidlist: fid_list,
        type: type,
      };
      let downloadUrl = `https://pan.baidu.com/api/download?bdstoken=${query.bdstoken}&web=5&app_id=250528&logid=${query.logid}=&channel=chunlei&clienttype=5`
      $.ajax({
        url: downloadUrl,
        async: false,
        method: 'POST',
        data: params,
        success: function (response) {
          result = response;
        }
      });
      return result;
    }

    function getDownloadLinkWithRESTAPIBaidu(path) {
      let link = restAPIUrl + 'file?method=download&path=' + encodeURIComponent(path) + '&app_id=' + secretCode;
      return link;
    }

    function getDownloadLinkWithClientAPI(path, cb) {
      let result;
      let url = clientAPIUrl + 'file?method=locatedownload&app_id=' + secretCode + '&ver=4.0&path=' + encodeURIComponent(path);

      GM_xmlhttpRequest({
        method: "POST",
        url: url,
        headers: {
          "User-Agent": userAgent,
        },
        onload: function (res) {
          if (res.status === 200) {
            result = JSON.parse(res.responseText);
            if (result.error_code == undefined) {
              if (result.urls == undefined) {
                result.errno = 2;
              } else {
                $.each(result.urls, function (index, element) {
                  result.urls[index].url = element.url.replace('\\', '');
                });
                result.errno = 0;
              }
            } else if (result.error_code == 31066) {
              result.errno = 1;
            } else {
              result.errno = -1;
            }
          } else {
            result = {};
            result.errno = -1;
          }
          cb(result);
        }
      });
    }

    function execDownload(link) {
      clog("下载链接：" + link);
      GM_openInTab(link, {active: true});
      //$('#helperdownloadiframe').attr('src', link);
    }

    function createIframe() {
      let $div = $('<div class="helper-hide" style="padding:0;margin:0;display:block"></div>');
      let $iframe = $('<iframe src="javascript:;" id="helperdownloadiframe" style="display:none"></iframe>');
      $div.append($iframe);
      $('body').append($div);

    }
  }

  //分享页面的下载助手
  function PanShareHelper() {
    let yunData, sign, timestamp, bdstoken, channel, clienttype, web, app_id, logid, encrypt, product, uk,
        primaryid, fid_list, extra, shareid;
    let vcode;
    let shareType, buttonTarget, currentPath, list_grid_status, observer, dialog, vcodeDialog;
    let fileList = [], selectFileList = [];
    let panAPIUrl = location.protocol + "//" + location.host + "/api/";
    let panHighAPIUrl = location.protocol + "//" + location.host + "/share/download?";
    let shareListUrl = location.protocol + "//" + location.host + "/share/list";

    this.init = function () {
      if (GM_getValue('SETTING_P')) {
        getShareCode();
      }
      yunData = unsafeWindow.yunData;
      clog('初始化信息:', yunData);
      if (yunData === undefined || yunData.FILEINFO == null) {
        clog('页面未正常加载，或者百度已经更新！');
        return;
      }
      initParams();
      addButton();
      dialog = new Dialog({addCopy: false});
      vcodeDialog = new VCodeDialog(refreshVCode, confirmClick);
      createIframe();
      registerVcode();

      if (!isSingleShare()) {
        registerEventListener();
        createObserver();
      }

      clog('下载助理加载成功！当前版本：', version);
    };

    function initParams() {
      shareType = getShareType();
      sign = yunData.SIGN;
      timestamp = yunData.TIMESTAMP;
      bdstoken = yunData.MYBDSTOKEN;
      channel = 'chunlei';
      clienttype = 0;
      web = 1;
      app_id = secretCode;
      logid = getLogID();
      encrypt = 0;
      product = 'share';
      primaryid = yunData.SHARE_ID;
      uk = yunData.SHARE_UK;

      if (shareType == 'secret') {
        extra = getExtra();
      }
      if (isSingleShare()) {
        let obj = {};
        if (yunData.CATEGORY == 2) {
          obj.filename = yunData.FILENAME;
          obj.path = yunData.PATH;
          obj.fs_id = yunData.FS_ID;
          obj.isdir = 0;
        } else {
          obj.filename = yunData.FILEINFO[0].server_filename,
              obj.path = yunData.FILEINFO[0].path,
              obj.fs_id = yunData.FILEINFO[0].fs_id,
              obj.isdir = yunData.FILEINFO[0].isdir;
        }
        selectFileList.push(obj);
      } else {
        shareid = yunData.SHARE_ID;
        currentPath = getPath();
        list_grid_status = getListGridStatus();
        fileList = getFileList();
      }
    }

    function getShareCode() {
      let hash = location.hash && /^#([a-zA-Z0-9]{4})$/.test(location.hash) && RegExp.$1,
          input = $('.pickpw input[tabindex="1"]'),
          btn = $('.pickpw a.g-button'),
          inputarea = $('.pickpw .input-area'),
          tip = $('<div style="margin:-8px 0 10px ;color: #ff5858">正在获取提取码</div>'),
          surl = (location.href.match(/\/init\?(?:surl|shareid)=((?:\w|-)+)/) || location.href.match(/\/s\/1((?:\w|-)+)/))[1];
      if (!input || !btn) {
        return;
      }
      inputarea.prepend(tip);
      if (hash) {
        tip.text('发现提取码，已自动为您填写');
        setTimeout(function () {
              input.val(hash);
              btn.click();
            },
            1e3);
      }

      $.ajax({
        method: 'GET',
        url: 'https://api.baiduyun.wiki/reset/' + surl,
        success: function (res) {
          if (res.link) {
            GM_xmlhttpRequest({
              method: 'GET',
              url: res.link,
              onload: function (xhr) {
                let result = JSON.parse(xhr.responseText);
                if (result.access_code) {
                  tip.text('发现提取码，已自动为您填写');
                  input.val(result.access_code);//填写密码
                  setTimeout(function () {
                    btn.click();
                    //showReferer(result.referrer);
                  }, 500);
                } else {
                  tip.text('未发现提取码，请手动填写');
                }
              }
            });
          } else {
            tip.text('未发现提取码，请手动填写');
          }
        },
        error: function (res) {
          tip.text('连接服务器失败，请手动填写');
        }
      });
    }

    function showReferer(referrer) {
      if (typeof referrer !== 'object') return false;
      let ref = Object.values(referrer);
      let temp = {};
      let refs = ref.reduce((preVal, curVal) => {
        temp[curVal.title] ? '' : temp[curVal.title] = true && preVal.push(curVal);
        return preVal;
      }, []);

      let ins = setInterval(function () {
        if ($('.slide-show-header').length > 0) {
          clearInterval(ins);
          $.each(refs, function (index, element) {
            if (element.title != "undefined") {
              let $div = $('<a style="display: block;margin-top: 7px;overflow: hidden;text-overflow: ellipsis;white-space: nowrap;" href="' + element.url + '" target="_blank">【来源】：' + element.title + '</a>');
              $('.slide-show-header').append($div);
            }
          });
        }
      }, 500);
    }

    //判断分享类型（public或者secret）
    function getShareType() {
      return yunData.SHARE_PUBLIC === 1 ? 'public' : 'secret';
    }

    //判断是单个文件分享还是文件夹或者多文件分享
    function isSingleShare() {
      return yunData.getContext === undefined ? true : false;
    }

    //判断是否为自己的分享链接
    function isSelfShare() {
      return yunData.MYSELF == 1 ? true : false;
    }

    function getExtra() {
      let seKey = decodeURIComponent(getCookie('BDCLND'));
      return '{' + '"sekey":"' + seKey + '"' + "}";
    }

    //获取当前目录
    function getPath() {
      let hash = location.hash;
      let regx = new RegExp("path=([^&]*)(&|$)", 'i');
      let result = hash.match(regx);
      return decodeURIComponent(result[1]);
    }

    //获取当前的视图模式
    function getListGridStatus() {
      let status = 'list';
      if ($('.list-switched-on').length > 0) {
        status = 'list';
      } else if ($('.grid-switched-on').length > 0) {
        status = 'grid';
      }
      return status;
    }

    //添加下载助手按钮
    function addButton() {
      if (isSingleShare()) {
        $('div.slide-show-right').css('width', '500px');
        $('div.frame-main').css('width', '96%');
        $('div.share-file-viewer').css('width', '740px').css('margin-left', 'auto').css('margin-right', 'auto');
      } else
        $('div.slide-show-right').css('width', '500px');
      let $dropdownbutton = $('<span class="g-dropdown-button"></span>');
      let $dropdownbutton_a = $('<a class="g-button g-button-blue" style="width: 114px;" data-button-id="b200" data-button-index="200" href="javascript:;"></a>');
      let $dropdownbutton_a_span = $('<span class="g-button-right"><em class="icon icon-speed" title="百度网盘下载助理"></em><span class="text" style="width: 60px;">下载助理</span></span>');
      let $dropdownbutton_span = $('<span class="menu" style="width:auto;z-index:41"></span>');

      let $saveButton = $('<a data-menu-id="b-menu207" class="g-button-menu" href="javascript:;">保存后下载</a>');
      let $saveSettingButton = $('<a data-menu-id="b-menu207" class="g-button-menu" href="javascript:;" style="opacity: 0.8;">自定义保存路径</a>');
      let $downloadButton = $('<a data-menu-id="b-menu207" class="g-button-menu" href="javascript:;">直接下载</a>');
      let $linkButton = $('<a data-menu-id="b-menu208" class="g-button-menu" href="javascript:;">显示直链</a>');
      let $highButton = $('<a data-menu-id="b-menu209" class="g-button-menu" style="color: #F24C43;font-weight: 700;" href="javascript:;">极速下载</a>');

      $dropdownbutton_span.append($downloadButton).append($linkButton)/*.append($ariclinkButton)*/.append($highButton).append($saveButton);
      //$dropdownbutton_span.append($saveButton)/*.append($saveSettingButton)*/.append($github);
      $dropdownbutton_a.append($dropdownbutton_a_span);
      $dropdownbutton.append($dropdownbutton_a).append($dropdownbutton_span);

      $dropdownbutton.hover(function () {
        $dropdownbutton.toggleClass('button-open');
      });
      $saveButton.click(saveButtonClick);
      $saveSettingButton.click(saveSettingButtonClick);
      $downloadButton.click(downloadButtonClick);
      $linkButton.click(linkButtonClick);
      $highButton.click(highButtonClick);

      $('div.module-share-top-bar div.bar div.x-button-box').append($dropdownbutton);
    }

    function createDir() {
      let query = {
        shareid: shareid,
        from: yunData.SHARE_UK,
        bdstoken: yunData.MYBDSTOKEN,
        logid: getLogID(),
      };
      let params = {
        path: savePath,
        isdir: 1,
        size: '',
        block_list: [],
        method: 'post',
        dataType: 'json'
      };
      let createAPIUrl = `https://pan.baidu.com/api/create?a=commit&channel=chunlei&app_id=250528&web=1&app_id=250528&bdstoken=${query.bdstoken}&logid=${query.logid}&clienttype=0`;

      $.ajax({
        url: createAPIUrl,
        async: false,
        method: 'POST',
        data: params,
        success: function (res) {
          if (res.errno === 0) {
            swal('目录创建成功！');
            saveButtonClick();
          } else {
            swal('目录创建失败，请前往我的网盘页面手动创建！');
          }
        }
      });
    }

    function saveButtonClick() {
      if (bdstoken === null) {
        swal(errorMsg.unlogin);
        return false;
      }
      if (selectFileList.length === 0) {
        swal(errorMsg.unselected);
        return;
      }
      if (isSelfShare()) {
        swal({
          title: "提示",
          text: '自己分享的文件请到网盘中下载！',
          buttons: {confirm: {text: "打开网盘", value: 'confirm'}}
        }).then((value) => {
          if (value === 'confirm') {
            location.href = 'https://pan.baidu.com/disk/home#/all?path=%2F&vmode=list';
          }
        });
        return;
      }
      let fsidlist = [];
      $.each(selectFileList, function (i, val) {
        fsidlist.push(val['fs_id']);
      });
      let query = {
        shareid: yunData.SHARE_ID,
        from: yunData.SHARE_UK,
        bdstoken: yunData.MYBDSTOKEN,
        logid: getLogID(),
      };
      let params = {
        path: GM_getValue('savePath'),
        fsidlist: JSON.stringify(fsidlist)
      };

      let saveAPIUrl = `https://pan.baidu.com/share/transfer?shareid=${query.shareid}&from=${query.from}&ondup=newcopy&async=1&channel=chunlei&web=1&app_id=250528&bdstoken=${query.bdstoken}&logid=${query.logid}&clienttype=0`;

      $.ajax({
        url: saveAPIUrl,
        async: false,
        method: 'POST',
        data: params,
        success: function (res) {
          if (res.errno === 0) {
            swal({
              title: "提示",
              text: '文件已保存至我的网盘，请再网盘中使用下载助手下载！',
              buttons: {confirm: {text: "打开网盘", value: 'confirm'}}
            }).then((value) => {
              if (value === 'confirm') {
                location.href = 'https://pan.baidu.com/disk/home#/all?vmode=list&path=' + encodeURIComponent(savePath);
              }
            });
          } else if (res.errno === 2) {
            swal({
              title: "提示",
              text: '保存目录不存在，是否先创建该目录？',
              buttons: {confirm: {text: "创建目录", value: 'confirm'}}
            }).then((value) => {
              if (value === 'confirm') {
                createDir();
              }
            });
          } else {
            swal('保存失败，请手动保存');
          }
        }
      });
    }

    function saveSettingButtonClick() {
      let str = prompt("请输入保存路径，例如/我的资源", savePath);
      if (str === null) return;
      if (/^\//.test(str)) {
        GM_setValue('savePath', str);
        swal({
          title: "提示",
          text: '路径设置成功！点击确定后立即生效',
          buttons: {confirm: {text: "确定", value: 'confirm'}}
        }).then((value) => {
          if (value === 'confirm') {
            history.go(0);
          }
        });
      } else {
        swal('请输入正确的路径，例如/PanHelper');
      }
    }

    function highButtonClick() {
      /*if (bdstoken !== null) {
        swal('请退出当前账号或使用浏览器小号/隐私模式获取不限速链接！！！');
        return false;
      }*/

      clog('选中文件列表：', selectFileList);
      if (selectFileList.length === 0) {
        swal(errorMsg.unselected);
        return false;
      }
      if (selectFileList[0].isdir == 1) {
        swal(errorMsg.toobig);
      }
      if (selectFileList.length > 1) {
        swal('一次只能勾选一个文件');
        return false;
      }

      getHighDownloadLink();
    }

    function createIframe() {
      let $div = $('<div class="helper-hide" style="padding:0;margin:0;display:block"></div>');
      let $iframe = $('<iframe src="javascript:;" id="helperdownloadiframe" style="display:none"></iframe>');
      $div.append($iframe);
      $('body').append($div);
    }

    function registerEventListener() {
      registerHashChange();
      registerListGridStatus();
      registerCheckbox();
      registerAllCheckbox();
      registerFileSelect();
    }

    function registerVcode() {
      $(document).on('click', '#changeCode', function () {
        getHighDownloadLink();
      });
    }

    //监视地址栏#标签变化
    function registerHashChange() {
      window.addEventListener('hashchange', function (e) {
        list_grid_status = getListGridStatus();
        if (currentPath == getPath()) {

        } else {
          currentPath = getPath();
          refreshFileList();
          refreshSelectFileList();
        }
      });
    }

    function refreshFileList() {
      fileList = getFileList();
    }

    function refreshSelectFileList() {
      selectFileList = [];
    }

    //监视视图变化
    function registerListGridStatus() {
      getListGridStatus();
    }

    //监视文件选择框
    function registerCheckbox() {
      list_grid_status = getListGridStatus();
      let $checkbox = $('span.' + classMap['checkbox']);
      if (list_grid_status == 'grid') {
        $checkbox = $('.' + classMap['chekbox-grid']);
      }
      $checkbox.each(function (index, element) {
        $(element).on('click', function (e) {
          let $parent = $(this).parent();
          let filename;
          let isActive;

          if (list_grid_status == 'list') {
            filename = $('.file-name div.text a', $parent).attr('title');
            isActive = $(this).parents('dd').hasClass('JS-item-active');
          } else if (list_grid_status == 'grid') {
            filename = $('div.file-name a', $parent).attr('title');
            isActive = !$(this).hasClass('JS-item-active');
          }

          if (isActive) {
            clog('取消选中文件：' + filename);
            for (let i = 0; i < selectFileList.length; i++) {
              if (selectFileList[i].filename == filename) {
                selectFileList.splice(i, 1);
              }
            }
          } else {
            clog('选中文件: ' + filename);
            $.each(fileList, function (index, element) {
              if (element.server_filename == filename) {
                let obj = {
                  filename: element.server_filename,
                  path: element.path,
                  fs_id: element.fs_id,
                  isdir: element.isdir
                };
                selectFileList.push(obj);
              }
            });
          }
        });
      });
    }

    function unregisterCheckbox() {
      let $checkbox = $('span.' + classMap['checkbox']);
      $checkbox.each(function (index, element) {
        $(element).unbind('click');
      });
    }

    //监视全选框
    function registerAllCheckbox() {
      let $checkbox = $('div.' + classMap['col-item'] + '.' + classMap['check']);
      $checkbox.each(function (index, element) {
        $(element).bind('click', function (e) {
          let $parent = $(this).parent();
          if ($parent.hasClass(classMap['checked'])) {
            clog('取消全选');
            selectFileList = [];
          } else {
            clog('全部选中');
            selectFileList = [];
            $.each(fileList, function (index, element) {
              let obj = {
                filename: element.server_filename,
                path: element.path,
                fs_id: element.fs_id,
                isdir: element.isdir
              };
              selectFileList.push(obj);
            });
          }
        });
      });
    }

    function unregisterAllCheckbox() {
      let $checkbox = $('div.' + classMap['col-item'] + '.' + classMap['check']);
      $checkbox.each(function (index, element) {
        $(element).unbind('click');
      });
    }

    //监视单个文件选中
    function registerFileSelect() {
      let $dd = $('div.' + classMap['list-view'] + ' dd');
      $dd.each(function (index, element) {
        $(element).bind('click', function (e) {
          let nodeName = e.target.nodeName.toLowerCase();
          if (nodeName != 'span' && nodeName != 'a' && nodeName != 'em') {
            selectFileList = [];
            let filename = $('div.file-name div.text a', $(this)).attr('title');
            clog('选中文件：' + filename);
            $.each(fileList, function (index, element) {
              if (element.server_filename == filename) {
                let obj = {
                  filename: element.server_filename,
                  path: element.path,
                  fs_id: element.fs_id,
                  isdir: element.isdir
                };
                selectFileList.push(obj);
              }
            });
          }
        });
      });
    }

    function unregisterFileSelect() {
      let $dd = $('div.' + classMap['list-view'] + ' dd');
      $dd.each(function (index, element) {
        $(element).unbind('click');
      });
    }

    //监视文件列表显示变化
    function createObserver() {
      let MutationObserver = window.MutationObserver;
      let options = {
        'childList': true
      };
      observer = new MutationObserver(function (mutations) {
        unregisterCheckbox();
        unregisterAllCheckbox();
        unregisterFileSelect();
        registerCheckbox();
        registerAllCheckbox();
        registerFileSelect();
      });

      let list_view = document.querySelector('.' + classMap['list-view']);
      let grid_view = document.querySelector('.' + classMap['grid-view']);

      observer.observe(list_view, options);
      observer.observe(grid_view, options);
    }

    //获取文件信息列表
    function getFileList() {
      let result = [];
      if (getPath() == '/') {
        result = yunData.FILEINFO;
      } else {
        logid = getLogID();
        let params = {
          uk: uk,
          shareid: shareid,
          order: 'other',
          desc: 1,
          showempty: 0,
          web: web,
          dir: getPath(),
          t: Math.random(),
          bdstoken: bdstoken,
          channel: channel,
          clienttype: clienttype,
          app_id: app_id,
          logid: logid
        };
        $.ajax({
          url: shareListUrl,
          method: 'GET',
          async: false,
          data: params,
          success: function (response) {
            if (response.errno === 0) {
              result = response.list;
            }
          }
        });
      }
      return result;
    }

    function downloadButtonClick() {
      if (bdstoken === null) {
        swal(errorMsg.unlogin);
        return false;
      }
      clog('选中文件列表：', selectFileList);
      if (selectFileList.length === 0) {
        swal(errorMsg.unselected);
        return false;
      }
      if (selectFileList.length > 1) {
        swal(errorMsg.morethan);
        return false;
      }

      if (selectFileList[0].isdir == 1) {
        swal(errorMsg.dir);
        return false;
      }
      buttonTarget = 'download';
      let downloadLink = getDownloadLink();

      if (downloadLink === undefined) return;

      if (downloadLink.errno == -20) {
        vcode = getVCode();
        if (vcode.errno !== 0) {
          swal('获取验证码失败！');
          return;
        }
        vcodeDialog.open(vcode);
      } else if (downloadLink.errno == 112) {
        swal('页面过期，请刷新重试');

      } else if (downloadLink.errno === 0) {
        let link = downloadLink.list[0].dlink;
        execDownload(link);
      } else {
        swal(errorMsg.fail);

      }
    }

    //获取验证码
    function getVCode() {
      let url = panAPIUrl + 'getvcode';
      let result;
      logid = getLogID();
      let params = {
        prod: 'pan',
        t: Math.random(),
        bdstoken: bdstoken,
        channel: channel,
        clienttype: clienttype,
        web: web,
        app_id: app_id,
        logid: logid
      };
      $.ajax({
        url: url,
        method: 'GET',
        async: false,
        data: params,
        success: function (response) {
          result = response;
        }
      });
      return result;
    }

    //刷新验证码
    function refreshVCode() {
      vcode = getVCode();
      $('#dialog-img').attr('src', vcode.img);
    }

    //验证码确认提交
    function confirmClick() {
      let val = $('#dialog-input').val();
      if (val.length === 0) {
        $('#dialog-err').text('请输入验证码');
        return;
      } else if (val.length < 4) {
        $('#dialog-err').text('验证码输入错误，请重新输入');
        return;
      }
      let result = getDownloadLinkWithVCode(val);
      if (result.errno == -20) {
        vcodeDialog.close();
        $('#dialog-err').text('验证码输入错误，请重新输入');
        refreshVCode();
        if (!vcode || vcode.errno !== 0) {
          swal('获取验证码失败！');
          return;
        }
        vcodeDialog.open();
      } else if (result.errno === 0) {
        vcodeDialog.close();
        if (buttonTarget == 'download') {
          if (result.list.length > 1 || result.list[0].isdir == 1) {
            swal(errorMsg.morethan);
            return false;
          }
          let link = result.list[0].dlink;
          execDownload(link);
        } else if (buttonTarget == 'link') {
          let tip = '支持使用IDM批量下载，需升级 <a href="https://www.baiduyun.wiki/zh-cn/assistant.html">[百度网盘万能助手]</a> 至v2.0.1';
          dialog.open({
            title: '下载链接（仅显示文件链接）',
            type: 'shareLink',
            list: result.list,
            tip: tip,
            showcopy: true
          });
        }
      } else {
        swal('发生错误！');
      }
    }

    //生成下载用的fid_list参数
    function getFidList() {
      let fidlist = [];
      $.each(selectFileList, function (index, element) {
        fidlist.push(element.fs_id);
      });
      return '[' + fidlist + ']';
    }

    function linkButtonClick() {
      if (bdstoken === null) {
        swal(errorMsg.unlogin);
        return false;
      }
      clog('选中文件列表：', selectFileList);
      if (selectFileList.length === 0) {
        swal(errorMsg.unselected);
        return false;
      }
      if (selectFileList[0].isdir == 1) {
        swal(errorMsg.dir);
        return false;
      }

      buttonTarget = 'link';
      let downloadLink = getDownloadLink();

      if (downloadLink === undefined) return;

      if (downloadLink.errno == -20) {
        vcode = getVCode();
        if (!vcode || vcode.errno !== 0) {
          swal('获取验证码失败！');
          return false;
        }
        vcodeDialog.open(vcode);
      } else if (downloadLink.errno == 112) {
        swal('页面过期，请刷新重试');
        return false;
      } else if (downloadLink.errno === 0) {
        let tip = '支持使用IDM批量下载，需升级 <a href="https://www.baiduyun.wiki/zh-cn/assistant.html">[百度网盘万能助手]</a> 至v2.0.1';
        dialog.open({
          title: '下载链接（仅显示文件链接）',
          type: 'shareLink',
          list: downloadLink.list,
          tip: tip,
          showcopy: true
        });
      } else {
        swal(errorMsg.fail);
      }
    }

    //获取下载链接
    function getDownloadLink() {
      if (bdstoken === null) {
        swal(errorMsg.unlogin);
        return '';
      } else {
        let result;
        if (isSingleShare) {
          fid_list = getFidList();
          logid = getLogID();
          let url = panAPIUrl + 'sharedownload?sign=' + sign + '&timestamp=' + timestamp + '&bdstoken=' + bdstoken + '&channel=' + channel + '&clienttype=' + clienttype + '&web=' + web + '&app_id=' + app_id + '&logid=' + logid;
          let params = {
            encrypt: encrypt,
            product: product,
            uk: uk,
            primaryid: primaryid,
            fid_list: fid_list
          };
          if (shareType == 'secret') {
            params.extra = extra;
          }
          /*if (selectFileList[0].isdir == 1 || selectFileList.length > 1) {
            params.type = 'batch';
          }*/
          $.ajax({
            url: url,
            method: 'POST',
            async: false,
            data: params,
            success: function (response) {
              result = response;
            }
          });
        }
        return result;
      }
    }

    //获取高速下载链接
    function getHighDownloadLink() {
      if (isSingleShare) {
        let high = {
          bdstoken: null,
          web: 5,
          app_id: 250528,
          logid: getLogID(),
          channel: 'chunlei',
          clienttype: 5,
          uk: yunData.SHARE_UK,
          shareid: yunData.SHARE_ID,
          fid_list: getFidList(),
          sign: yunData.SIGN,
          timestamp: yunData.TIMESTAMP,
        };
        let url = panHighAPIUrl + 'sign=' + high.sign + '&timestamp=' + high.timestamp + '&bdstoken=' + high.bdstoken + '&channel=' + high.channel + '&clienttype=' + high.clienttype + '&web=' + high.web + '&app_id=' + high.app_id + '&logid=' + high.logid + '&fid_list=' + high.fid_list + '&uk=' + high.uk + '&shareid=' + high.shareid;
        $.ajax({
          url: url,
          method: 'GET',
          async: false,
          success: function (res) {
            if (res.errno == 0) {
              let tip = '左键或右键下载';
              dialog.open({title: '不限速链接（仅支持单文件）', type: 'highLink', list: res.dlink, tip: tip});
            }
            if (res.errno == -19) {  //验证码
              vcode = res.vcode;
              createVCode(res.img);
            }
          }
        });
      }
    }

    function createVCode(img) {
      removeVCode();
      let $vbody = $('<div class="v-body" style="display: flex;justify-content: center;align-items: center;"></div>');
      let $input = $('<input type="text" placeholder="验证码" id="input-code" maxlength="4" style="width: 80px;height: 40px;margin: 0;padding: 5px;box-sizing: border-box;border: 1px solid #ddd;font-size: 14px;">');
      let $img = $('<img id="img-code" alt="验证码获取中" src="' + img + '" style="margin: 0 8px;width: 120px;height: 40px;;box-sizing: border-box;">');
      let $button = $('<a href="javascript:;" id="changeCode">换一张</a>');

      $vbody.append($input).append($img).append($button);
      swal({
        title: "请输入验证码",
        content: $vbody[0],
        buttons: true,
      }).then((handle) => {
        if (handle) {
          submitVCode();
        }
      });
    }

    function removeVCode() {
      $('.v-body').remove();
    }

    //提交验证码
    function submitVCode() {
      let input = $('#input-code').val();
      if (input.length === 0) {
        swal({
          text: "请输入验证码",
          icon: "error",
        });
        return;
      } else if (input.length < 4) {
        swal({
          text: "验证码输入错误，请重新输入",
          icon: "error",
        });
        return;
      }

      let high = {
        bdstoken: null,
        web: 5,
        app_id: 250528,
        logid: getLogID(),
        channel: 'chunlei',
        clienttype: 5,
        uk: yunData.SHARE_UK,
        shareid: yunData.SHARE_ID,
        fid_list: getFidList(),
        sign: yunData.SIGN,
        timestamp: yunData.TIMESTAMP,
      };
      let url = panHighAPIUrl + 'sign=' + high.sign + '&timestamp=' + high.timestamp + '&bdstoken=' + high.bdstoken + '&channel=' + high.channel + '&clienttype=' + high.clienttype + '&web=' + high.web + '&app_id=' + high.app_id + '&logid=' + high.logid + '&fid_list=' + high.fid_list + '&uk=' + high.uk + '&shareid=' + high.shareid + '&input=' + input + '&vcode=' + vcode;

      $.ajax({
        url: url,
        method: 'GET',
        async: false,
        success: function (res) {
          if (res.errno == 0) {
            removeVCode();
            let tip = '左键或右键下载';
            dialog.open({title: '不限速链接（仅支持单文件）', type: 'highLink', list: res.dlink, tip: tip});
          }
          if (res.errno == -19) {  //验证码
            swal({
              text: "验证码输入错误，请重新输入",
              icon: "error",
            });
            vcode = res.vcode;
            createVCode(res.img);
          }
        }
      });
    }

    //有验证码输入时获取下载链接
    function getDownloadLinkWithVCode(vcodeInput) {
      let result;
      if (isSingleShare) {
        fid_list = getFidList();
        let url = panAPIUrl + 'sharedownload?sign=' + sign + '&timestamp=' + timestamp + '&bdstoken=' + bdstoken + '&channel=' + channel + '&clienttype=' + clienttype + '&web=' + web + '&app_id=' + app_id + '&logid=' + logid;
        let params = {
          encrypt: encrypt,
          product: product,
          vcode_input: vcodeInput,
          vcode_str: vcode.vcode,
          uk: uk,
          primaryid: primaryid,
          fid_list: fid_list
        };
        if (shareType == 'secret') {
          params.extra = extra;
        }
        /*if (selectFileList[0].isdir == 1 || selectFileList.length > 1) {
          params.type = 'batch';
        }*/
        $.ajax({
          url: url,
          method: 'POST',
          async: false,
          data: params,
          success: function (response) {
            result = response;
          }
        });
      }
      return result;
    }

    function execDownload(link) {
      clog('下载链接：' + link);
      GM_openInTab(link, {active: true});
      //$('#helperdownloadiframe').attr('src', link);
    }
  }

  function base64Encode(t) {
    let a, r, e, n, i, s, o = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (e = t.length, r = 0, a = ""; e > r;) {
      if (n = 255 & t.charCodeAt(r++), r == e) {
        a += o.charAt(n >> 2);
        a += o.charAt((3 & n) << 4);
        a += "==";
        break;
      }
      if (i = t.charCodeAt(r++), r == e) {
        a += o.charAt(n >> 2);
        a += o.charAt((3 & n) << 4 | (240 & i) >> 4);
        a += o.charAt((15 & i) << 2);
        a += "=";
        break;
      }
      s = t.charCodeAt(r++);
      a += o.charAt(n >> 2);
      a += o.charAt((3 & n) << 4 | (240 & i) >> 4);
      a += o.charAt((15 & i) << 2 | (192 & s) >> 6);
      a += o.charAt(63 & s);
    }
    return a;
  }

  function detectPage() {
    let regx = /[\/].+[\/]/g;
    let page = location.pathname.match(regx);
    return page[0].replace(/\//g, '');
  }

  function getCookie(e) {
    let o, t;
    let n = document, c = decodeURI;
    return n.cookie.length > 0 && (o = n.cookie.indexOf(e + "="), -1 != o) ? (o = o + e.length + 1, t = n.cookie.indexOf(";", o), -1 == t && (t = n.cookie.length), c(n.cookie.substring(o, t))) : "";
  }

  function setCookie(key, value, t) {
    let oDate = new Date();  //创建日期对象
    oDate.setTime(oDate.getTime() + t * 60 * 1000); //设置过期时间
    document.cookie = key + '=' + value + ';expires=' + oDate.toGMTString();  //设置cookie的名称，数值，过期时间
  }

  function removeCookie(key) {
    setCookie(key, '', -1);  //cookie的过期时间设为昨天
  }

  function getLogID() {
    let name = "BAIDUID";
    let u = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/~！@#￥%……&";
    let d = /[\uD800-\uDBFF][\uDC00-\uDFFFF]|[^\x00-\x7F]/g;
    let f = String.fromCharCode;

    function l(e) {
      if (e.length < 2) {
        let n = e.charCodeAt(0);
        return 128 > n ? e : 2048 > n ? f(192 | n >>> 6) + f(128 | 63 & n) : f(224 | n >>> 12 & 15) + f(128 | n >>> 6 & 63) + f(128 | 63 & n);
      }
      let n = 65536 + 1024 * (e.charCodeAt(0) - 55296) + (e.charCodeAt(1) - 56320);
      return f(240 | n >>> 18 & 7) + f(128 | n >>> 12 & 63) + f(128 | n >>> 6 & 63) + f(128 | 63 & n);
    }

    function g(e) {
      return (e + "" + Math.random()).replace(d, l);
    }

    function m(e) {
      let n = [0, 2, 1][e.length % 3];
      let t = e.charCodeAt(0) << 16 | (e.length > 1 ? e.charCodeAt(1) : 0) << 8 | (e.length > 2 ? e.charCodeAt(2) : 0);
      let o = [u.charAt(t >>> 18), u.charAt(t >>> 12 & 63), n >= 2 ? "=" : u.charAt(t >>> 6 & 63), n >= 1 ? "=" : u.charAt(63 & t)];
      return o.join("");
    }

    function h(e) {
      return e.replace(/[\s\S]{1,3}/g, m);
    }

    function p() {
      return h(g((new Date()).getTime()));
    }

    function w(e, n) {
      return n ? p(String(e)).replace(/[+\/]/g, function (e) {
        return "+" == e ? "-" : "_";
      }).replace(/=/g, "") : p(String(e));
    }

    return w(getCookie(name));
  }

  function Dialog() {
    let linkList = [];
    let showParams;
    let dialog, shadow;

    function createDialog() {
      let screenWidth = document.body.clientWidth;
      let dialogLeft = screenWidth > 800 ? (screenWidth - 800) / 2 : 0;
      let $dialog_div = $('<div class="dialog" style="width: 800px; top: 0px; bottom: auto; left: ' + dialogLeft + 'px; right: auto; display: hidden; visibility: visible; z-index: 52;"></div>');
      let $dialog_header = $('<div class="dialog-header"><h3><span class="dialog-title" style="display:inline-block;width:740px;white-space:nowrap;overflow-x:hidden;text-overflow:ellipsis"></span></h3></div>');
      let $dialog_control = $('<div class="dialog-control"><span class="dialog-icon dialog-close">×</span></div>');
      let $dialog_body = $('<div class="dialog-body" style="max-height:450px;overflow-y:auto;padding:0 20px;"></div>');
      let $dialog_tip = $('<div class="dialog-tip" style="padding-left:20px;background-color:#fff;border-top: 1px solid #c4dbfe;color: #dc373c;"><p></p></div>');

      $dialog_div.append($dialog_header.append($dialog_control)).append($dialog_body);

      let $dialog_button = $('<div class="dialog-button" style="display:none"></div>');
      let $dialog_button_div = $('<div style="display:table;margin:auto"></div>');
      let $dialog_copy_button = $('<button id="dialog-copy-button" style="display:none;width: 100px; margin: 5px 0 10px 0; cursor: pointer; background: #cc3235; border: none; height: 30px; color: #fff; border-radius: 3px;">复制全部链接</button>');
      let $dialog_edit_button = $('<button id="dialog-edit-button" style="display:none">编辑</button>');
      let $dialog_exit_button = $('<button id="dialog-exit-button" style="display:none">退出</button>');

      $dialog_button_div.append($dialog_copy_button).append($dialog_edit_button).append($dialog_exit_button);
      $dialog_button.append($dialog_button_div);
      $dialog_div.append($dialog_button);

      $dialog_copy_button.click(function () {
        let content = '';
        if (showParams.type == 'batch') {
          $.each(linkList, function (index, element) {
            if (element.downloadlink == 'error')
              return;
            if (index == linkList.length - 1)
              content += element.downloadlink;
            else
              content += element.downloadlink + '\r\n';
          });
        } else if (showParams.type == 'shareLink') {
          $.each(linkList, function (index, element) {
            if (element.dlink == 'error')
              return;
            if (index == linkList.length - 1)
              content += element.dlink;
            else
              content += element.dlink + '\r\n';
          });
        }
        GM_setClipboard(content, 'text');
        if (content != '') {
          swal('已将链接复制到剪贴板！');
        } else {
          swal('复制失败，请手动复制！');
        }
      });

      $dialog_edit_button.click(function () {
        let $dialog_textarea = $('div.dialog-body textarea[name=dialog-textarea]', dialog);
        let $dialog_item = $('div.dialog-body div', dialog);
        $dialog_item.hide();
        $dialog_copy_button.hide();
        $dialog_edit_button.hide();
        $dialog_textarea.show();
        $dialog_radio_div.show();
        $dialog_exit_button.show();
      });

      $dialog_exit_button.click(function () {
        let $dialog_textarea = $('div.dialog-body textarea[name=dialog-textarea]', dialog);
        let $dialog_item = $('div.dialog-body div', dialog);
        $dialog_textarea.hide();
        $dialog_radio_div.hide();
        $dialog_item.show();
        $dialog_exit_button.hide();
        $dialog_copy_button.show();
        $dialog_edit_button.show();
      });

      $dialog_div.append($dialog_tip);
      $('body').append($dialog_div);
      $dialog_control.click(dialogControl);
      return $dialog_div;
    }

    function createShadow() {
      let $shadow = $('<div class="dialog-shadow" style="position: fixed; left: 0px; top: 0px; z-index: 50; background: rgb(0, 0, 0) none repeat scroll 0% 0%; opacity: 0.5; width: 100%; height: 100%; display: none;"></div>');
      $('body').append($shadow);
      return $shadow;
    }

    this.open = function (params) {
      showParams = params;
      linkList = [];
      if (params.type == 'link') {
        linkList = params.list.urls;
        $('div.dialog-header h3 span.dialog-title', dialog).text(params.title + "：" + params.list.filename);
        $.each(params.list.urls, function (index, element) {
          element.url = replaceLink(element.url);
          let $div = $('<div><div style="width:30px;float:left">' + element.rank + ':</div><div style="white-space:nowrap;overflow:hidden;text-overflow:ellipsis"><a href="' + element.url + '">' + element.url + '</a></div></div>');

          $('div.dialog-body', dialog).append($div);
        });
      }
      if (params.type == 'batch') {
        linkList = params.list;
        $('div.dialog-header h3 span.dialog-title', dialog).text(params.title);
        if (params.showall) {
          $.each(params.list, function (index, element) {
            let $item_div = $('<div class="item-container" style="overflow:hidden;text-overflow:ellipsis;white-space:nowrap"></div>');
            let $item_name = $('<div style="width:100px;float:left;overflow:hidden;text-overflow:ellipsis" title="' + element.filename + '">' + element.filename + '</div>');
            let $item_sep = $('<div style="width:12px;float:left"><span>：</span></div>');
            let $item_link_div = $('<div class="item-link" style="float:left;width:618px;"></div>');
            let $item_first;
             $item_first = $('<div class="item-first" style="overflow:hidden;text-overflow:ellipsis"><a href="' + element.downloadlink + '">' + element.downloadlink + '</a></div>');

            $item_link_div.append($item_first);
            $.each(params.alllist[index].links, function (n, item) {
              let $item;
              if (element.downloadlink == item.url)
                return;
              item.url = replaceLink(item.url);
              $item = $('<div class="item-ex" style="display:none;overflow:hidden;text-overflow:ellipsis"><a href="' + item.url + '">' + item.url + '</a></div>');

              $item_link_div.append($item);
            });
            let $item_ex = $('<div style="width:15px;float:left;cursor:pointer;text-align:center;font-size:16px"><span>+</span></div>');
            $item_div.append($item_name).append($item_sep).append($item_link_div).append($item_ex);
            $item_ex.click(function () {
              let $parent = $(this).parent();
              $parent.toggleClass('showall');
              if ($parent.hasClass('showall')) {
                $(this).text('-');
                $('div.item-link div.item-ex', $parent).show();
              } else {
                $(this).text('+');
                $('div.item-link div.item-ex', $parent).hide();
              }
            });
            $('div.dialog-body', dialog).append($item_div);
          });
        } else {
          $.each(params.list, function (index, element) {
            let $div;
            $div = $('<div style="overflow:hidden;text-overflow:ellipsis;white-space:nowrap"><div style="width:100px;float:left;overflow:hidden;text-overflow:ellipsis" title="' + element.filename + '">' + element.filename + '</div><span>：</span><a href="' + element.downloadlink + '">' + element.downloadlink + '</a></div>');
            $('div.dialog-body', dialog).append($div);
          });
        }
      }
      if (params.type == 'shareLink') {
        linkList = params.list;
        $('div.dialog-header h3 span.dialog-title', dialog).text(params.title);
        $.each(params.list, function (index, element) {
          element.dlink = replaceLink(element.dlink);
          if (element.isdir == 1) return;
          let $div = $('<div style="overflow:hidden;text-overflow:ellipsis;white-space:nowrap"><div style="width:100px;float:left;overflow:hidden;text-overflow:ellipsis" title="' + element.server_filename + '">' + element.server_filename + '</div><span>：</span><a href="' + element.dlink + '">' + element.dlink + '</a></div>');
          $('div.dialog-body', dialog).append($div);
        });
      }

      if (params.type == 'highLink') {
        let link = params.list;
        link = replaceLink(link);
        $('div.dialog-header h3 span.dialog-title', dialog).text(params.title);
        let $div = $('<div style="overflow:hidden;text-overflow:ellipsis;white-space:nowrap"><div style="width:100px;float:left;overflow:hidden;text-overflow:ellipsis">普通链接</div><span>：</span><a href="' + link + '">' + link + '</a></div>');
		$('div.dialog-body', dialog).append($div);
      }

      if (params.tip) {
        $('div.dialog-tip p', dialog).html(params.tip);
      }

      if (params.showcopy) {
        $('div.dialog-button', dialog).show();
        $('div.dialog-button button#dialog-copy-button', dialog).show();
      }
      if (params.showedit) {
        $('div.dialog-button', dialog).show();
        $('div.dialog-button button#dialog-edit-button', dialog).show();
        let $dialog_textarea = $('<textarea name="dialog-textarea" style="display:none;resize:none;width:758px;height:300px;white-space:pre;word-wrap:normal;overflow-x:scroll"></textarea>');
        let content = '';
        if (showParams.type == 'batch') {
          $.each(linkList, function (index, element) {
            if (element.downloadlink == 'error')
              return;
            if (index == linkList.length - 1)
              content += element.downloadlink;
            else
              content += element.downloadlink + '\r\n';
          });
        } else if (showParams.type == 'link') {
          $.each(linkList, function (index, element) {
            if (element.url == 'error')
              return;
            if (index == linkList.length - 1)
              content += element.url;
            else
              content += element.url + '\r\n';
          });
        }
        $dialog_textarea.val(content);
        $('div.dialog-body', dialog).append($dialog_textarea);
      }

      shadow.show();
      dialog.show();
    };

    this.close = function () {
      dialogControl();
    };

    function dialogControl() {
      $('div.dialog-body', dialog).children().remove();
      $('div.dialog-header h3 span.dialog-title', dialog).text('');
      $('div.dialog-tip p', dialog).text('');
      $('div.dialog-button', dialog).hide();
      $('div.dialog-radio input[type=radio][name=showmode][value=multi]', dialog).prop('checked', true);
      $('div.dialog-radio', dialog).hide();
      $('div.dialog-button button#dialog-copy-button', dialog).hide();
      $('div.dialog-button button#dialog-edit-button', dialog).hide();
      $('div.dialog-button button#dialog-exit-button', dialog).hide();
      dialog.hide();
      shadow.hide();
    }

    dialog = createDialog();
    shadow = createShadow();
  }

  function VCodeDialog(refreshVCode, confirmClick) {
    let dialog, shadow;

    function createDialog() {
      let screenWidth = document.body.clientWidth;
      let dialogLeft = screenWidth > 520 ? (screenWidth - 520) / 2 : 0;
      let $dialog_div = $('<div class="dialog" id="dialog-vcode" style="width:520px;top:0px;bottom:auto;left:' + dialogLeft + 'px;right:auto;display:none;visibility:visible;z-index:52"></div>');
      let $dialog_header = $('<div class="dialog-header"><h3><span class="dialog-header-title"><em class="select-text">提示</em></span></h3></div>');
      let $dialog_control = $('<div class="dialog-control"><span class="dialog-icon dialog-close icon icon-close"><span class="sicon">x</span></span></div>');
      let $dialog_body = $('<div class="dialog-body"></div>');
      let $dialog_body_div = $('<div style="text-align:center;padding:22px"></div>');
      let $dialog_body_download_verify = $('<div class="download-verify" style="margin-top:10px;padding:0 28px;text-align:left;font-size:12px;"></div>');
      let $dialog_verify_body = $('<div class="verify-body">请输入验证码：</div>');
      let $dialog_input = $('<input id="dialog-input" type="text" style="padding:3px;width:85px;height:23px;border:1px solid #c6c6c6;background-color:white;vertical-align:middle;" class="input-code" maxlength="4">');
      let $dialog_img = $('<img id="dialog-img" class="img-code" style="margin-left:10px;vertical-align:middle;" alt="点击换一张" src="" width="100" height="30">');
      let $dialog_refresh = $('<a href="javascript:;" style="text-decoration:underline;" class="underline">换一张</a>');
      let $dialog_err = $('<div id="dialog-err" style="padding-left:84px;height:18px;color:#d80000" class="verify-error"></div>');
      let $dialog_footer = $('<div class="dialog-footer g-clearfix"></div>');
      let $dialog_confirm_button = $('<a class="g-button g-button-blue" data-button-id="" data-button-index href="javascript:;" style="padding-left:36px"><span class="g-button-right" style="padding-right:36px;"><span class="text" style="width:auto;">确定</span></span></a>');
      let $dialog_cancel_button = $('<a class="g-button" data-button-id="" data-button-index href="javascript:;" style="padding-left: 36px;"><span class="g-button-right" style="padding-right: 36px;"><span class="text" style="width: auto;">取消</span></span></a>');

      $dialog_header.append($dialog_control);
      $dialog_verify_body.append($dialog_input).append($dialog_img).append($dialog_refresh);
      $dialog_body_download_verify.append($dialog_verify_body).append($dialog_err);
      $dialog_body_div.append($dialog_body_download_verify);
      $dialog_body.append($dialog_body_div);
      $dialog_footer.append($dialog_confirm_button).append($dialog_cancel_button);
      $dialog_div.append($dialog_header).append($dialog_body).append($dialog_footer);
      $('body').append($dialog_div);

      $dialog_control.click(dialogControl);
      $dialog_img.click(refreshVCode);
      $dialog_refresh.click(refreshVCode);
      $dialog_input.keypress(function (event) {
        if (event.which == 13)
          confirmClick();
      });
      $dialog_confirm_button.click(confirmClick);
      $dialog_cancel_button.click(dialogControl);
      $dialog_input.click(function () {
        $('#dialog-err').text('');
      });
      return $dialog_div;
    }

    this.open = function (vcode) {
      if (vcode)
        $('#dialog-img').attr('src', vcode.img);
      dialog.show();
      shadow.show();
    };
    this.close = function () {
      dialogControl();
    };
    dialog = createDialog();
    shadow = $('div.dialog-shadow');

    function dialogControl() {
      $('#dialog-img', dialog).attr('src', '');
      $('#dialog-err').text('');
      dialog.hide();
      shadow.hide();
    }
  }

  function PanPlugin() {
    this.init = function () {
      GM_setValue('current_version', version);
      loadPanhelper();
      initParams();
      createHelp();
      createMenu();
    };

    function loadPanhelper() {
      switch (detectPage()) {
        case 'disk':
          let panHelper = new PanHelper();
          panHelper.init();
          return;
        case 'share':
        case 's':
          let panShareHelper = new PanShareHelper();
          panShareHelper.init();
          return;
        default:
          return;
      }
    }
	
    function createHelp() {
      setTimeout(() => {
        let topbar = $('.' + classMap['header']);
        let toptemp = $('<span class="cMEMEF" node-type="help-author" style="opacity: .5" ><a href="https://www.baiduyun.wiki/zh-cn/" >教程</a><i class="find-light-icon" style="display: inline;background-color: #009fe8;"></i></span>');
        topbar.append(toptemp);
      }, 5000);
    }

    function createMenu() {
      GM_registerMenuCommand('网盘脚本配置', function () {
        if (GM_getValue('SETTING_P') === undefined) {
          GM_setValue('SETTING_P', true);
        }
        let dom = '';
        if (GM_getValue('SETTING_P')) {
          dom += '<label style="display:flex;align-items: center;justify-content: space-between;padding-top: 20px;">自动填写提取码<input type="checkbox" id="S-P" checked style="width: 16px;height: 16px;"></label>';
        } else {
          dom += '<label style="display:flex;align-items: center;justify-content: space-between;padding-top: 20px;">自动填写提取码<input type="checkbox" id="S-P" style="width: 16px;height: 16px;"></label>';
        }
        dom = '<div>' + dom + '</div>';
        let $dom = $(dom);
        swal({content: $dom[0]});
      });
      $(document).on('change', '#S-P', function () {
        GM_setValue('SETTING_P', $(this)[0].checked);
      });
    }

    function initParams() {
      classMap['default-dom'] = ($('.icon-upload').parent().parent().parent().parent().parent().attr('class'));
      classMap['bar'] = ($('.icon-upload').parent().parent().parent().parent().attr('class'));

      let script = document.createElement("script");
      script.type = "text/javascript";
      script.async = true;
      script.src = "https://js.users.51.la/19988117.js";
      document.getElementsByTagName("head")[0].appendChild(script);

      //解决https无法加载http资源的问题
      let oMeta = document.createElement('meta');
      oMeta.httpEquiv = 'Content-Security-Policy';
      oMeta.content = 'upgrade-insecure-requests';
      document.getElementsByTagName('head')[0].appendChild(oMeta);
    }
  }

  $(function () {
    let plugin = new PanPlugin();
    plugin.init();
  });
})();
