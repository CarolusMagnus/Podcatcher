﻿using System;
using System.Net;
using System.Diagnostics;
using System.Collections.ObjectModel;
using System.Windows.Navigation;
using System.Windows;
using Microsoft.Phone.Controls;
using System.Windows.Controls;
using Coding4Fun.Phone.Controls;
using Podcatcher.ViewModels;

namespace Podcatcher
{
    public delegate void SubscriptionManagerHandler(object source, SubscriptionManagerArgs e);

    public class SubscriptionManagerArgs
    {
    }

    public class PodcastSubscriptionsManager
    {
        /************************************* Public implementations *******************************/

        public event SubscriptionManagerHandler OnPodcastChannelFinished;
        public event SubscriptionManagerHandler OnPodcastChannelFinishedWithError;

        public static PodcastSubscriptionsManager getInstance()
        {
            if (m_instance == null)
            {
                m_instance = new PodcastSubscriptionsManager();
            }

            return m_instance;
        }

        public void addSubscriptionFromURL(string podcastRss)
        {
            // DEBUG
            if (String.IsNullOrEmpty(podcastRss))
            {
                podcastRss = "http://leo.am/podcasts/twit";
            }

            if (podcastRss.StartsWith("http://") == false)
            {
                podcastRss = podcastRss.Insert(0, "http://");
            }

            Uri podcastRssUri;
            try
            {
                podcastRssUri = new Uri(podcastRss);
            }
            catch (UriFormatException)
            {
                Debug.WriteLine("ERROR: Cannot add podcast from that URL.");
                OnPodcastChannelFinishedWithError(this, null);
                return;
            }

            WebClient wc = new WebClient();
            wc.DownloadStringCompleted += new DownloadStringCompletedEventHandler(wc_DownloadPodcastRSSCompleted);
            wc.DownloadStringAsync(podcastRssUri);

            Debug.WriteLine("Fetching podcast from URL: " + podcastRss.ToString());
        }


        /************************************* Private implementation *******************************/
        #region privateImplementations
        private static PodcastSubscriptionsManager m_instance = null;
        private PodcastSqlModel m_podcastsModel = null;

        private PodcastSubscriptionsManager()
        {
            m_podcastsModel = PodcastSqlModel.getInstance();
        }

        private void wc_DownloadPodcastRSSCompleted(object sender, DownloadStringCompletedEventArgs e)
        {
            if (e.Error != null
                || e.Cancelled)
            {
                Debug.WriteLine("ERROR: Web request failed. Message: " + e.Error.Message);
                OnPodcastChannelFinishedWithError(this, null);
                return;
            }
                
            PodcastSubscriptionModel podcastModel = PodcastFactory.podcastModelFromRSS(e.Result);            
            Debug.WriteLine("Got new podcast, name: " + podcastModel.PodcastName);

            podcastModel.PodcastLogoLocalLocation = localLogoFileName(podcastModel);

            OnPodcastChannelFinished(this, null);

            m_podcastsModel.addSubscription(podcastModel);
        }

        private string localLogoFileName(PodcastSubscriptionModel podcastModel)
        {
            // Parse the filename of the logo from the remote URL.
            string localPath = podcastModel.PodcastLogoUrl.LocalPath;
            string podcastLogoFilename = localPath.Substring(localPath.LastIndexOf('/') + 1);
            string localPodcastLogoFilename = App.PODCAST_ICON_DIR + @"/" + podcastLogoFilename;

            Debug.WriteLine("Found icon filename: " + localPodcastLogoFilename);

            return localPodcastLogoFilename;
        }
        #endregion
    }
}